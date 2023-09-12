﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Plaza;

namespace Plaza
{
    public class Entity
    {
        public UInt64 Uuid;

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return InternalCalls.HasComponent(Uuid, componentType);
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;
            T component = new T() { Entity = this };
            component.Uuid = this.Uuid;
            return component;
        }
    }

    public abstract class Component
    {
        public UInt64 Uuid;
        public Entity Entity { get; internal set; }
    }



    public class Transform : Component 
    {
        public Vector3 Translation
        {
            get
            {
                InternalCalls.GetPositionCall(Entity.Uuid, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.SetPosition(Uuid, ref value);
            }
        }
        public Vector3 rotation;
        public Vector3 scale;

        public void MoveTowards(Vector3 vector3)
        {
            Plaza.InternalCalls.MoveTowards(this.Uuid, vector3);
        }
    }

    public class MeshRenderer : Component
    {
        public Vector3[] Vertices
        {
            get
            {
                return InternalCalls.MeshRenderer_GetVertices(Entity.Uuid);
            }
            set
            {
                InternalCalls.MeshRenderer_SetVertices(Uuid, value);
            }
        }

        public int[] Indices
        {
            get
            {
                return InternalCalls.MeshRenderer_GetIndices(Entity.Uuid);
            }
            set
            {
                InternalCalls.MeshRenderer_SetIndices(Uuid, value);
            }
        }

        public Vector3[] Normals
        {
            get
            {
                return InternalCalls.MeshRenderer_GetNormals(Entity.Uuid);
            }
            set
            {
                InternalCalls.MeshRenderer_SetNormals(Uuid, value);
            }
        }
        /*
        public Vector2[] Uvs
        {
            get
            {
                InternalCalls.MeshRenderer_GetUvs(Entity.Uuid, out Vector2[] uvs);
                return uvs;
            }
            set
            {
                InternalCalls.MeshRenderer_SetUvs(Uuid, ref value);
            }
        }

        */
    }

}
