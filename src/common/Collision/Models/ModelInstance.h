/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#ifndef _MODELINSTANCE_H_
#define _MODELINSTANCE_H_

#include <G3D/Matrix3.h>
#include <G3D/Vector3.h>
#include <G3D/AABox.h>
#include <G3D/Ray.h>

#include "Define.h"

namespace VMAP
{
    class WorldModel;
    struct AreaInfo;
    struct LocationInfo;
    enum class ModelIgnoreFlags : uint32;

    enum ModelFlags
    {
        MOD_M2 = 1,
        MOD_HAS_BOUND = 1 << 1,
        MOD_PARENT_SPAWN = 1 << 2
    };

    class COMMON_API ModelSpawn
    {
        public:
            //mapID, tileX, tileY, Flags, ID, Pos, Rot, Scale, Bound_lo, Bound_hi, name
            uint32 flags;
            uint16 adtId;
            uint32 ID;
            G3D::Vector3 iPos;
            G3D::Vector3 iRot;
            float iScale;
            G3D::AABox iBound;
            std::string name;
            bool operator==(const ModelSpawn &other) const { return ID == other.ID; }
            //uint32 hashCode() const { return ID; }
            // temp?
            const G3D::AABox& getBounds() const { return iBound; }

            static bool readFromFile(FILE* rf, ModelSpawn &spawn);
            static bool writeToFile(FILE* rw, const ModelSpawn &spawn);
    };

    class COMMON_API ModelInstance: public ModelSpawn
    {
        public:
            ModelInstance(): iInvScale(0.0f), iModel(nullptr) { }
            ModelInstance(const ModelSpawn &spawn, WorldModel* model);
            void setUnloaded() { iModel = nullptr; }
            bool intersectRay(const G3D::Ray& pRay, float& pMaxDist, bool pStopAtFirstHit, ModelIgnoreFlags ignoreFlags) const;
            void intersectPoint(const G3D::Vector3& p, AreaInfo &info) const;
            bool GetLocationInfo(const G3D::Vector3& p, LocationInfo &info) const;
            bool GetLiquidLevel(const G3D::Vector3& p, LocationInfo &info, float &liqHeight) const;
            WorldModel* getWorldModel() { return iModel; }
        protected:
            G3D::Matrix3 iInvRot;
            float iInvScale;
            WorldModel* iModel;
    };
} // namespace VMAP

#endif // _MODELINSTANCE
