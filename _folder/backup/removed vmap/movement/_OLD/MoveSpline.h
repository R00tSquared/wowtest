/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2015 Hellground <http://hellground.net/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef HELLGROUND_MOVEPLINE_H
#define HELLGROUND_MOVEPLINE_H

#include "spline.h"
#include "MoveSplineInitArgs.h"
#include "Log.h"

namespace Movement
{
    struct Location : public Vector3
    {
        Location() : orientation(0) {}
        Location(float x, float y, float z, float o) : Vector3(x,y,z), orientation(o) {}
        Location(const Vector3& v) : Vector3(v), orientation(0) {}
        Location(const Vector3& v, float o) : Vector3(v), orientation(o) {}

        float orientation;
    };

    // MoveSpline represents smooth catmullrom or linear curve and point that moves belong it
    // curve can be cyclic - in this case movement will be cyclic
    // point can have vertical acceleration motion componemt(used in fall, parabolic movement)
    class MoveSpline
    {
    public:
        typedef Spline<int32> MySpline;
        enum UpdateResult{
            Result_None         = 0x01,
            Result_Arrived      = 0x02,
            Result_NextCycle    = 0x04,
            Result_NextSegment  = 0x08,
        };
        #pragma region fields
        friend class PacketBuilder;
    protected:
        MySpline        spline;

        FacingInfo      facing;

        uint32          m_Id;

        MoveSplineFlag  splineflags;

        int32           time_passed;
        int32           point_Idx;
        int32           point_Idx_offset;

        void init_spline(const MoveSplineInitArgs& args);
    protected:

        const MySpline::ControlArray& getPath() const { return spline.getPoints();}
        void computeFallElevation(float& el) const;

        UpdateResult _updateState(int32& ms_time_diff);
        int32 next_timestamp() const { return spline.length(point_Idx+1);}
        int32 timeElapsed() const { return Duration() - time_passed;}
        int32 timePassed() const { return time_passed;}

    public:
        const MySpline& _Spline() const { return spline;}
        int32 _currentSplineIdx() const { return point_Idx;}
        void _Finalize();
        void _Interrupt() { splineflags.done = true;}

        #pragma endregion
    public:

        void Initialize(const MoveSplineInitArgs&);
        bool Initialized() const { return !spline.empty();}

        explicit MoveSpline();

        template<class UpdateHandler>
        void updateState(int32 difftime, UpdateHandler& handler)
        {
            ASSERT(Initialized());
            do
                handler(_updateState(difftime));
            while(difftime > 0);
        }

        void updateState(int32 difftime)
        {
            ASSERT(Initialized());
            do _updateState(difftime);
            while(difftime > 0);
        }

        Location ComputePosition() const;

        uint32 GetId() const { return m_Id;}
        bool Finalized() const { return splineflags.done; }
        bool isCyclic() const { return splineflags.cyclic;}
        const Vector3 FinalDestination() const { return Initialized() ? spline.getPoint(spline.last()) : Vector3();}
        const Vector3 CurrentDestination() const { return Initialized() ? spline.getPoint(point_Idx+1) : Vector3();}
        int32 currentPathIdx() const;

        int32 segment_time_elapsed() const { return next_timestamp()-time_passed;}
        int32 Duration() const { return spline.length();}

        std::string ToString() const;
    };
}
#endif // MANGOSSERVER_MOVEPLINE_H
