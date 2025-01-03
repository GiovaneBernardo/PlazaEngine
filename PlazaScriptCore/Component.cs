﻿using System;
using System.CodeDom;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Win32;
using Plaza;

namespace Plaza
{
    public class Entity
    {

        public UInt64 Uuid;
        public string Name
        {
            get
            {
                return InternalCalls.EntityGetName(this.Uuid);
            }
            set
            {
                InternalCalls.EntitySetName(this.Uuid, value);
            }
        }

        public Entity parent
        {
            get
            {
                return new Entity(InternalCalls.EntityGetParent(this.Uuid));
            }
            set
            {
                InternalCalls.EntitySetParent(this.Uuid, value.Uuid);
            }
        }

        public List<Entity> Children
        {
            get
            {
                List<UInt64> childrenUuid = InternalCalls.EntityGetChildren(Uuid);
                List<Entity> childrenList = new List<Entity>();
                foreach (UInt64 child in childrenUuid)
                {
                    childrenList.Add(new Entity(child));
                }
                return childrenList;
            }
        }

        public static Entity NewEntity()
        {
            return new Entity(InternalCalls.NewEntity());
        }

        public Entity()
        {

        }

        public Entity(ulong uuid)
        {
            Uuid = uuid;
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            if (componentType.IsSubclassOf(typeof(ScriptComponent)))
            {
                return InternalCalls.HasScript(Uuid, componentType);
            }
            return InternalCalls.HasComponent(Uuid, componentType);
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;
            if (typeof(T).IsSubclassOf(typeof(ScriptComponent)))
            {
                T component = new T() { Entity = this };
                component.Uuid = this.Uuid;
                return component;
            }
            else
            {
                T component = new T() { Entity = this };
                component.Uuid = this.Uuid;
                return component;
            }
        }

        public T AddComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
                return null;
            T component = new T() { Entity = this };
            component.Uuid = this.Uuid;
            InternalCalls.AddComponent(this.Uuid, typeof(T));
            return component;
        }

        public void RemoveComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return;
            InternalCalls.RemoveComponent(this.Uuid, typeof(T));
        }

        public T GetScript<T>() where T : Entity, new()
        {
            return InternalCalls.GetScript(this.Uuid) as T;
        }

        public bool HasScript<T>() where T : Entity, new()
        {
            Type scriptType = typeof(T);
            return InternalCalls.HasScript(Uuid, scriptType);
        }

        public T AddScript<T>() where T : Entity, new()
        {
            if (HasScript<T>())
                return null;
            T script = new T() { Uuid = this.Uuid };
            script.Uuid = this.Uuid;
            InternalCalls.AddScript(this.Uuid, typeof(T));
            return script as T;
        }

        public static Entity FindEntityByName(string name)
        {
            ulong entityUuid = InternalCalls.FindEntityByNameCall(name);
            if (entityUuid == 0)
                return null;
            return new Entity(entityUuid);
        }

        public static Entity Instantiate(Entity otherEntity)
        {
            return new Entity(InternalCalls.Instantiate(otherEntity.Uuid));
        }

        public void Delete()
        {
            InternalCalls.EntityDelete(this.Uuid);
        }
    }

    public abstract class Component : Entity
    {
        public UInt64 Uuid;
        public Entity Entity { get; internal set; }
        public bool Enabled
        {
            get
            {
                return InternalCalls.Component_IsEnabled(this.Uuid, this.GetType());
            }
            set
            {
                InternalCalls.Component_SetEnabled(this.Uuid, this.GetType(), value);
            }
        }
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
        public Quaternion Rotation
        {
            get
            {
                InternalCalls.GetRotationQuaternionCall(Entity.Uuid, out Vector4 rotation);
                return new Quaternion(rotation.X, rotation.Y, rotation.Z, rotation.W);
            }
            set
            {
                Vector4 quaternion = new Vector4(value.x, value.y, value.z, value.w);
                InternalCalls.SetRotationQuaternion(Uuid, ref quaternion);
            }
        }
        /*        public Vector3 Rotation
                {
                    get
                    {
                        InternalCalls.GetRotationCall(Entity.Uuid, out Vector3 rotation);
                        return rotation;
                    }
                    set
                    {
                        InternalCalls.SetRotation(Uuid, ref value);
                    }
                }*/
        public Vector3 Scale
        {
            get
            {
                InternalCalls.GetScaleCall(Entity.Uuid, out Vector3 rotation);
                return rotation;
            }
            set
            {
                InternalCalls.SetScaleCall(Uuid, ref value);
            }
        }

        public Vector3 ForwardVector
        {
            get
            {
                InternalCalls.Transform_GetForwardVector(Entity.Uuid, out Vector3 vec);
                return vec;
            }
        }

        public Vector3 UpVector
        {
            get
            {
                InternalCalls.Transform_GetUpVector(Entity.Uuid, out Vector3 vec);
                return vec;
            }
        }

        public Vector3 LeftVector
        {
            get
            {
                InternalCalls.Transform_GetLeftVector(Entity.Uuid, out Vector3 vec);
                return vec;
            }
        }

        public Matrix4 WorldMatrix
        {
            get
            {
                float[] receivedMatrix = InternalCalls.Transform_GetWorldMatrix(Entity.Uuid);
                float[,] matrix4x4 = new float[4, 4];
                int matInd = 0;
                for (int i = 0; i < 4; i++)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        matrix4x4[i, j] = receivedMatrix[matInd];
                        matInd++;
                    }
                }
                return new Matrix4(matrix4x4);
            }
            set
            {
                //InternalCalls.Transform_SetWorldMatrix(Uuid, ref value);
            }
        }

        public void MoveTowards(Vector3 vector3)
        {
            Plaza.InternalCalls.MoveTowards(this.Uuid, vector3);
        }

        public Vector3 MoveTowardsReturn(Vector3 vector3)
        {
            Plaza.InternalCalls.MoveTowardsReturn(this.Uuid, vector3, out Vector3 outVector);
            return outVector;
        }
    }

    public class Mesh
    {
        public Mesh()
        {

        }
        public UInt64 uuid;
        public Vector3[] Vertices;

        public int[] Indices;

        public Vector3[] Normals;
        public Vector2[] Uvs;

        public Mesh InvertTriangleOrder()
        {
            Mesh newMesh = this;
            newMesh.Indices = this.Indices.Reverse<int>().ToArray();
            return newMesh;
        }
    }

    public class MeshRenderer : Component
    {
        public Mesh mesh
        {
            set
            {
                InternalCalls.MeshRenderer_SetMesh(Uuid, value.Vertices, value.Indices, value.Normals, value.Uvs);
            }
        }

        public void SetMaterial(UInt64 materialUuid)
        {
            InternalCalls.MeshRenderer_SetMaterial(Uuid, materialUuid);
        }
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

        public Vector2[] Uvs
        {
            get
            {
                return InternalCalls.MeshRenderer_GetUvs(Entity.Uuid);
            }
            set
            {
                InternalCalls.MeshRenderer_SetUvs(Uuid, value);
            }
        }
    }

    public class AnimationComponent : Component
    {

    }

    #region Rigid Body
    public struct Angular
    {
        public bool X, Y, Z;
    }
    public enum ForceMode
    {
        FORCE,
        IMPULSE,
        VELOCITY_CHANGE,
        ACCELERATION
    }

    public class RigidBody : Component
    {
        public float drag
        {
            get
            {
                return InternalCalls.RigidBody_GetDrag(Uuid);
            }
            set
            {
                InternalCalls.RigidBody_SetDrag(Uuid, value);
            }
        }
        public Vector3 velocity
        {
            get
            {
                InternalCalls.RigidBody_GetVelocity(Uuid, out Vector3 value);
                return value;
            }
            set
            {
                InternalCalls.RigidBody_SetVelocity(Uuid, ref value);
            }
        }
        public void ApplyForce(Vector3 force)
        {
            InternalCalls.RigidBody_ApplyForce(Uuid, ref force);
        }
        public void AddForce(Vector3 force, ForceMode mode = ForceMode.FORCE, bool autowake = true)
        {
            InternalCalls.RigidBody_AddForce(Uuid, ref force, mode, autowake);
        }
        public void AddTorque(Vector3 torque, ForceMode mode = ForceMode.FORCE, bool autowake = true)
        {
            InternalCalls.RigidBody_AddTorque(Uuid, ref torque, mode, autowake);
        }
        public void LockAngular(Axis axis, bool value)
        {
            InternalCalls.RigidBody_LockAngular(this.Uuid, axis, value);
        }

        public Angular IsAngularLocked()
        {
            return InternalCalls.RigidBody_IsAngularLocked(this.Uuid);
        }
    }
    #endregion

    #region Collider

    public class Collision
    {
        public Entity collidedEntity;
    }
    public enum ColliderShapeEnum
    {
        BOX,
        SPHERE,
        CAPSULE,
        PLANE,
        CYLINDER,
        MESH,
        CONVEX_MESH,
        HEIGHT_FIELD
    };
    public class Collider : Component
    {
        public void AddShape(ColliderShapeEnum shape)
        {
            InternalCalls.Collider_AddShape(this.Uuid, shape);
        }

        public void AddShape(ColliderShapeEnum shape, Mesh mesh)
        {
            InternalCalls.Collider_AddShapeMesh(this.Uuid, shape, mesh);
        }

        public void AddShape(ColliderShapeEnum shape, float[,] heightData, int size)
        {
            InternalCalls.Collider_AddShapeHeightField(this.Uuid, shape, heightData, size);
        }
    }
    #endregion Collider

    #region Character Controller
    public class CharacterController : Component
    {
        public void Move(Vector3 position, float minimumDistance, bool followOrientation, float elapsedTime)
        {
            InternalCalls.CharacterController_MoveCall(this.Uuid, position, minimumDistance, followOrientation, elapsedTime);
        }
    }
    #endregion Character Controller

    #region Camera
    public class Camera : Component
    {
        public Matrix4 ProjectionMatrix
        {
            get
            {
                InternalCalls.Camera_GetProjectionMatrix(this.Uuid, out Vector4 row0, out Vector4 row1, out Vector4 row2, out Vector4 row3);

                Matrix4 matrix = new Matrix4();

                Vector4[] rows = { row0, row1, row2, row3 };
                for (int i = 0; i < 4; ++i)
                {
                    matrix.data[i, 0] = rows[i].X;
                    matrix.data[i, 1] = rows[i].Y;
                    matrix.data[i, 2] = rows[i].Z;
                    matrix.data[i, 3] = rows[i].W;
                }

                return matrix;
            }
        }
        public Matrix4 ViewMatrix
        {
            get
            {
                InternalCalls.Camera_GetViewMatrix(this.Uuid, out Vector4 row0, out Vector4 row1, out Vector4 row2, out Vector4 row3);

                Matrix4 matrix = new Matrix4();

                Vector4[] rows = { row0, row1, row2, row3 };
                for (int i = 0; i < 4; ++i)
                {
                    matrix.data[i, 0] = rows[i].X;
                    matrix.data[i, 1] = rows[i].Y;
                    matrix.data[i, 2] = rows[i].Z;
                    matrix.data[i, 3] = rows[i].W;
                }

                return matrix;
            }
        }

        public Matrix4 InverseMVP
        {
            get
            {
                InternalCalls.Camera_GetInverseMVP(this.Uuid, out Vector4 row0, out Vector4 row1, out Vector4 row2, out Vector4 row3);

                Matrix4 matrix = new Matrix4();

                Vector4[] rows = { row0, row1, row2, row3 };
                for (int i = 0; i < 4; ++i)
                {
                    matrix.data[i, 0] = rows[i].X;
                    matrix.data[i, 1] = rows[i].Y;
                    matrix.data[i, 2] = rows[i].Z;
                    matrix.data[i, 3] = rows[i].W;
                }

                return matrix;
            }
        }

        public Matrix4 InverseProjection
        {
            get
            {
                InternalCalls.Camera_GetInverseProjection(this.Uuid, out Vector4 row0, out Vector4 row1, out Vector4 row2, out Vector4 row3);

                Matrix4 matrix = new Matrix4();

                Vector4[] rows = { row0, row1, row2, row3 };
                for (int i = 0; i < 4; ++i)
                {
                    matrix.data[i, 0] = rows[i].X;
                    matrix.data[i, 1] = rows[i].Y;
                    matrix.data[i, 2] = rows[i].Z;
                    matrix.data[i, 3] = rows[i].W;
                }

                return matrix;
            }
        }

        public Matrix4 InverseView
        {
            get
            {
                InternalCalls.Camera_GetInverseView(this.Uuid, out Vector4 row0, out Vector4 row1, out Vector4 row2, out Vector4 row3);

                Matrix4 matrix = new Matrix4();

                Vector4[] rows = { row0, row1, row2, row3 };
                for (int i = 0; i < 4; ++i)
                {
                    matrix.data[i, 0] = rows[i].X;
                    matrix.data[i, 1] = rows[i].Y;
                    matrix.data[i, 2] = rows[i].Z;
                    matrix.data[i, 3] = rows[i].W;
                }

                return matrix;
            }
        }
    }
    #endregion Camera

    #region TextRenderer
    public class TextRenderer : Component
    {
        public string text
        {
            get
            {
                return InternalCalls.TextRenderer_GetText(this.Uuid);
            }
            set
            {
                InternalCalls.TextRenderer_SetText(this.Uuid, value);
            }
        }
        public Vector2 position
        {
            get
            {
                InternalCalls.TextRenderer_GetPosition(Uuid, out Vector2 position);
                return position;
            }
            set
            {
                InternalCalls.TextRenderer_SetPosition(Uuid, ref value);
            }
        }

        public float scale
        {
            get
            {
                return InternalCalls.TextRenderer_GetScale(Entity.Uuid);
            }
            set
            {
                InternalCalls.TextRenderer_SetScale(Uuid, value);
            }
        }
        public void SetText(string text = "", float x = 0, float y = 0, float scale = 1.0f, Vector4 color = default(Vector4))
        {
            InternalCalls.TextRenderer_SetFullText(Uuid, text, x, y, scale, ref color);
        }
    }
    #endregion TextRenderer

    #region Script Component
    public class ScriptComponent : Component
    {

    }

    public class Script : ScriptComponent { }


    #endregion Script Component

    #region AudioSource Component
    public class AudioSource : Component
    {

        public void Play()
        {
            InternalCalls.AudioSource_Play(this.Uuid);
        }
        public void Stop()
        {
            InternalCalls.AudioSource_Stop(this.Uuid);
        }
        public bool spatial
        {
            get
            {
                return InternalCalls.AudioSource_GetSpatial(this.Uuid);
            }
            set
            {
                InternalCalls.AudioSource_SetSpatial(this.Uuid, value);
            }
        }

        public float volume
        {
            get
            {
                return InternalCalls.AudioSource_GetVolume(this.Uuid);
            }
            set
            {
                InternalCalls.AudioSource_SetVolume(this.Uuid, value);
            }
        }

        public float pitch
        {
            get
            {
                return InternalCalls.AudioSource_GetPitch(this.Uuid);
            }
            set
            {
                InternalCalls.AudioSource_SetPitch(this.Uuid, value);
            }
        }
    }

    public enum GuiType
    {
        PL_GUI_RECTANGLE = 0,
        PL_GUI_BUTTON,
        PL_GUI_TEXT
    };

    public class GuiItem
    {
        public UInt64 GuiUuid = 0;

        public GuiItem() { }
        public GuiItem(ulong uuid)
        {
            GuiUuid = uuid;
        }

        public string GuiName
        {
            get
            {
                return InternalCalls.GuiComponent_GuiGetName(this.GuiComponentUuid, this.GuiUuid);
            }
        }
        public UInt64 GuiComponentUuid = 0;
        public UInt64 GuiParentUuid
        {
            get
            {
                return InternalCalls.GuiComponent_GuiGetParentUuid(this.GuiComponentUuid, this.GuiUuid);
            }
        }
        public List<UInt64> GuiChildren = new List<UInt64>();
        public GuiType GuiType        
        {
            get
            {
                return InternalCalls.GuiComponent_GuiGetType(this.GuiComponentUuid, this.GuiUuid);
            }
        }

        public Vector2 LocalPosition
        {
            get
            {
                InternalCalls.GuiComponent_GuiGetLocalPosition(this.GuiComponentUuid, this.GuiUuid, out Vector2 position);
                return position;
            }
            set
            {
                InternalCalls.GuiComponent_GuiSetLocalPosition(this.GuiComponentUuid, this.GuiUuid, ref value);
            }
        }
        public Vector2 LocalSize
        {
            get
            {
                InternalCalls.GuiComponent_GuiGetLocalSize(this.GuiComponentUuid, this.GuiUuid, out Vector2 size);
                return size;
            }
            set
            {
                InternalCalls.GuiComponent_GuiSetLocalSize(this.GuiComponentUuid, this.GuiUuid, ref value);
            }
        }
    }

    public class GuiRectangle : GuiItem
    {

    }

    public class GuiButton : GuiItem
    {
        public string Text 
        {
            set
            {
                InternalCalls.GuiComponent_GuiButtonSetText(this.GuiComponentUuid, this.GuiUuid, value);
            }
        }
        public float FontScale
        {
            set
            {
                InternalCalls.GuiComponent_GuiButtonSetScale(this.GuiComponentUuid, this.GuiUuid, value);
            }
        }

        public void AddCallback(UInt64 scriptUuid, string functionName)
        {
            InternalCalls.GuiComponent_GuiButtonAddCallback(this.GuiComponentUuid, this.GuiUuid, scriptUuid, functionName);
        }
    }


    public class GuiComponent : Component
    {
        //List<GuiItem> GuiItems = new List<GuiItem>();

        public T FindGuiByName<T>(string name) where T : GuiItem, new()
        {
            T gui = new T();
            gui.GuiUuid = InternalCalls.GuiComponent_GuiGetByName(this.Uuid, name);
            gui.GuiComponentUuid = this.Uuid;
            return gui;
        }
    }

    #endregion
}


