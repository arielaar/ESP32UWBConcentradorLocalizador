#include "PositioningManager.h"
#include <cmath> // Para fabs y sqrt

PositioningManager::PositioningManager(int minAnchors) : _minAnchors(minAnchors) {}

void PositioningManager::setAnchorPosition(uint16_t anchor_saddr, float x, float y, float z) {
    _anchorPositions[anchor_saddr] = {x, y, z};
}

Point PositioningManager::getLastTagPosition() const {
    return _lastTagPosition;
}

const std::map<uint16_t, DecodedAnchorReport_t>& PositioningManager::getAnchorDataMap() const {
    return _latestAnchorData;
}

void PositioningManager::addAnchorReport(const DecodedAnchorReport_t& report) {
    _latestAnchorData[report.anchor_saddr] = report;
    _sequenceData[report.seq][report.anchor_saddr] = report;

    if (_sequenceData[report.seq].size() >= _minAnchors) {
        DEBUG_PRINTF("\n[POS] Secuencia %u completa con %d anclas. Calculando posición...\n", report.seq, _sequenceData[report.seq].size());
        calculateTagPosition(report.seq);
        _sequenceData.erase(report.seq);
    }
}

void PositioningManager::calculateTagPosition(uint16_t sequence_id) {
    // Reúne los reportes de esta secuencia
    auto itSeq = _sequenceData.find(sequence_id);
    if (itSeq == _sequenceData.end()) return;

    const auto& anchorReadings = itSeq->second;
    const size_t M = anchorReadings.size();
    if (M < _minAnchors) {
        DEBUG_PRINTF("[POS] Faltan anclas: %u/%u.\n", (unsigned)M, (unsigned)_minAnchors);
        return;
    }

    // 1) Verificar que conocemos posiciones de TODAS las anclas del conjunto
    std::vector<uint16_t> saddrList; saddrList.reserve(M);
    for (auto const& kv : anchorReadings) {
        uint16_t saddr = kv.first;
        if (_anchorPositions.find(saddr) == _anchorPositions.end()) {
            DEBUG_PRINTF("[POS] Error: No se conoce la posición del ancla 0x%X\n", saddr);
            return;
        }
        saddrList.push_back(saddr);
    }

    // 2) Extraer posiciones (Ai) y distancias (ri) en arreglos ordenados
    struct ARow { double x,y,z; };
    std::vector<ARow> Apos;  Apos.reserve(M);
    std::vector<double> range; range.reserve(M);

    for (uint16_t saddr : saddrList) {
        const auto& P = _anchorPositions.at(saddr);   // Point {x,y,z}
        const auto& R = anchorReadings.at(saddr);      // DecodedAnchorReport_t (range_m, etc.)
        Apos.push_back({ (double)P.x, (double)P.y, (double)P.z });
        range.push_back((double)R.range_m);
    }

    // 3) Detectar si trabajamos en 2D (todas Z ~ iguales) o 3D
    auto z0 = Apos[0].z; 
    bool almost2D = true;
    for (size_t i=1;i<M;i++) if (fabs(Apos[i].z - z0) > 1e-3) { almost2D = false; break; }

    // 4) Elegir referencia: fila 0
    const double x0=Apos[0].x, y0=Apos[0].y, z0r=Apos[0].z;
    const double r0=range[0];

    // 5) Construir A y b para el sistema lineal (N-1 ecuaciones)
    const size_t rows = (M >= 2 ? M-1 : 0);
    if (rows == 0) { DEBUG_PRINTLN("[POS] Conjunto insuficiente."); return; }

    // Matrices pequeñas; resolvemos con normales e inversión cerrada 2x2 o 3x3
    if (almost2D) {
        // ----- Caso 2D: A es (rows)x2, p=[x,y]
        // A_i = 2*(Ai - A0) en x,y ; b_i = (||Ai||^2-||A0||^2) + (ri^2 - r0^2)
        double JTJ[2][2] = {{0,0},{0,0}};
        double JTb[2]    = {0,0};

        for (size_t i=1;i<M;i++) {
            const double dxi = Apos[i].x - x0;
            const double dyi = Apos[i].y - y0;
            // Ignoramos z en 2D, pero ojo: los rangos incluyen z real; esta es la aproximación estándar en planta.
            const double Ai2 = Apos[i].x*Apos[i].x + Apos[i].y*Apos[i].y;  // ||Ai||^2 en 2D
            const double A02 = x0*x0 + y0*y0;                              // ||A0||^2 en 2D
            const double bi  = (Ai2 - A02) + (range[i]*range[i] - r0*r0);

            const double a0 = 2.0*dxi;
            const double a1 = 2.0*dyi;

            // Acumular normales
            JTJ[0][0] += a0*a0; JTJ[0][1] += a0*a1;
            JTJ[1][0] += a1*a0; JTJ[1][1] += a1*a1;
            JTb[0]    += a0*bi; JTb[1]    += a1*bi;
        }

        // Resolver (JTJ) p = JTb (2x2)
        const double det = JTJ[0][0]*JTJ[1][1] - JTJ[0][1]*JTJ[1][0];
        if (fabs(det) < 1e-18) { DEBUG_PRINTLN("[POS] Geometría 2D mal condicionada."); return; }

        const double inv00 =  JTJ[1][1]/det;
        const double inv01 = -JTJ[0][1]/det;
        const double inv10 = -JTJ[1][0]/det;
        const double inv11 =  JTJ[0][0]/det;

        const double x = inv00*JTb[0] + inv01*JTb[1];
        const double y = inv10*JTb[0] + inv11*JTb[1];
        const double z = 0.0; // planta

        _lastTagPosition = { (float)x, (float)y, (float)z };

        // RMS de residuo (opcional)
        double rss=0; 
        for (size_t i=0;i<M;i++) {
            const double dx=x - Apos[i].x, dy=y - Apos[i].y;
            const double Ri = sqrt(dx*dx + dy*dy);
            const double res = Ri - range[i];
            rss += res*res;
        }
        const double rms = sqrt(rss / M);
        DEBUG_PRINTF("[POS] 2D OK (N=%u). Pos=(%.3f, %.3f) RMS=%.3f m\n", (unsigned)M, x, y, rms);
    } else {
        // ----- Caso 3D: A es (rows)x3, p=[x,y,z]
        double JTJ[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
        double JTb[3]    = {0,0,0};

        const double A02 = x0*x0 + y0*y0 + z0r*z0r;

        for (size_t i=1;i<M;i++) {
            const double dxi = Apos[i].x - x0;
            const double dyi = Apos[i].y - y0;
            const double dzi = Apos[i].z - z0r;

            const double Ai2 = Apos[i].x*Apos[i].x + Apos[i].y*Apos[i].y + Apos[i].z*Apos[i].z;
            const double bi  = (Ai2 - A02) + (range[i]*range[i] - r0*r0);

            const double a0 = 2.0*dxi;
            const double a1 = 2.0*dyi;
            const double a2 = 2.0*dzi;

            // Acumular normales JTJ += a*a^T ; JTb += a*bi
            JTJ[0][0]+=a0*a0; JTJ[0][1]+=a0*a1; JTJ[0][2]+=a0*a2;
            JTJ[1][0]+=a1*a0; JTJ[1][1]+=a1*a1; JTJ[1][2]+=a1*a2;
            JTJ[2][0]+=a2*a0; JTJ[2][1]+=a2*a1; JTJ[2][2]+=a2*a2;

            JTb[0]+=a0*bi; JTb[1]+=a1*bi; JTb[2]+=a2*bi;
        }

        // Inversión 3x3 por adjunta
        const double a=JTJ[0][0], b=JTJ[0][1], c=JTJ[0][2];
        const double d=JTJ[1][0], e=JTJ[1][1], f=JTJ[1][2];
        const double g=JTJ[2][0], h=JTJ[2][1], i=JTJ[2][2];

        const double A11 =  (e*i - f*h);
        const double A12 = -(b*i - c*h);
        const double A13 =  (b*f - c*e);
        const double A21 = -(d*i - f*g);
        const double A22 =  (a*i - c*g);
        const double A23 = -(a*f - c*d);
        const double A31 =  (d*h - e*g);
        const double A32 = -(a*h - b*g);
        const double A33 =  (a*e - b*d);

        const double det = a*A11 + b*A21 + c*A31;
        if (fabs(det) < 1e-18) { DEBUG_PRINTLN("[POS] Geometría 3D mal condicionada."); return; }

        const double inv[3][3] = {
            { A11/det, A12/det, A13/det },
            { A21/det, A22/det, A33/det },
            { A31/det, A32/det, A33/det }
        };

        const double x = inv[0][0]*JTb[0] + inv[0][1]*JTb[1] + inv[0][2]*JTb[2];
        const double y = inv[1][0]*JTb[0] + inv[1][1]*JTb[1] + inv[1][2]*JTb[2];
        const double z = inv[2][0]*JTb[0] + inv[2][1]*JTb[1] + inv[2][2]*JTb[2];

        _lastTagPosition = { (float)x, (float)y, (float)z };

        // RMS de residuo (opcional)
        double rss=0;
        for (size_t k=0;k<M;k++) {
            const double dx=x - Apos[k].x, dy=y - Apos[k].y, dz=z - Apos[k].z;
            const double Ri = sqrt(dx*dx + dy*dy + dz*dz);
            const double res = Ri - range[k];
            rss += res*res;
        }
        const double rms = sqrt(rss / M);
        DEBUG_PRINTF("[POS] 3D OK (N=%u). Pos=(%.3f, %.3f, %.3f) RMS=%.3f m\n", (unsigned)M, x, y, z, rms);
    }
}
