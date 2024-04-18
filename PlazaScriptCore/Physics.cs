﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Plaza
{
    public static class Physics
    {
        public struct RaycastHit {
            public UInt64 hitUuid;
            public Vector3 point;
            public Vector3 normal;
        }

        public static RaycastHit Raycast(Vector3 origin, Vector3 direction, float maxDistance, UInt64 ignoredUuid = 0)
        {
            InternalCalls.Physics_Raycast(origin, direction, maxDistance, out RaycastHit hit, ignoredUuid);
            return hit;
        }
    }
}
