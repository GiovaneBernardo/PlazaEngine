using System;
using System.Collections.Generic;
using System.Linq;
using Plaza;

namespace Plaza
{
    /// <summary>
    /// Represents an in-engine entity, which can have various component attachments.
    /// This class serves as a wrapper around lower-level calls (InternalCalls).
    /// </summary>
    public class Entity
    {
        public ulong Uuid;

        /// <summary>
        /// The name of this entity. Backed by an engine-level string retrieval.
        /// </summary>
        public string Name
        {
            get => InternalCalls.EntityGetName(Uuid);
            set => InternalCalls.EntitySetName(Uuid, value);
        }

        /// <summary>
        /// The parent entity in the scene hierarchy.
        /// </summary>
        public Entity Parent
        {
            get => new Entity(InternalCalls.EntityGetParent(Uuid));
            set => InternalCalls.EntitySetParent(Uuid, value.Uuid);
        }

        /// <summary>
        /// All immediate child entities in the scene hierarchy.
        /// </summary>
        public List<Entity> Children
        {
            get
            {
                var childrenUuidList = InternalCalls.EntityGetChildren(Uuid);
                var result = new List<Entity>(childrenUuidList.Count);
                foreach (var childUuid in childrenUuidList)
                {
                    result.Add(new Entity(childUuid));
                }
                return result;
            }
        }

        /// <summary>
        /// Creates a new entity in the engine and returns the wrapper.
        /// </summary>
        public static Entity CreateNew() => new Entity(InternalCalls.NewEntity());

        public Entity() { }

        public Entity(ulong uuid)
        {
            Uuid = uuid;
        }

        /// <summary>
        /// Checks if this entity has a specified component type.
        /// </summary>
        public bool HasComponent<T>() where T : Component, new()
        {
            var componentType = typeof(T);
            if (componentType.IsSubclassOf(typeof(ScriptComponent)))
            {
                return InternalCalls.HasScript(Uuid, componentType);
            }
            return InternalCalls.HasComponent(Uuid, componentType);
        }

        /// <summary>
        /// Retrieves a component of type T from this entity, or null if not found.
        /// </summary>
        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            // If it's a script component, or a normal component:
            T component = new T
            {
                Entity = this,
                Uuid = Uuid
            };
            return component;
        }

        /// <summary>
        /// Adds a new component of type T to this entity if it doesn't already have one.
        /// </summary>
        public T AddComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
                return null;

            var component = new T
            {
                Entity = this,
                Uuid = Uuid
            };
            InternalCalls.AddComponent(Uuid, typeof(T));
            return component;
        }

        /// <summary>
        /// Removes an existing component of type T from this entity if it is present.
        /// </summary>
        public void RemoveComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return;
            InternalCalls.RemoveComponent(Uuid, typeof(T));
        }

        /// <summary>
        /// Retrieves the user-defined script instance (if any).
        /// </summary>
        public T GetScript<T>() where T : Entity, new()
        {
            return InternalCalls.GetScript(Uuid) as T;
        }

        /// <summary>
        /// Checks if a user-defined script T is present on this entity.
        /// </summary>
        public bool HasScript<T>() where T : Entity, new()
        {
            var scriptType = typeof(T);
            return InternalCalls.HasScript(Uuid, scriptType);
        }

        /// <summary>
        /// Adds a user-defined script instance of type T to this entity.
        /// </summary>
        public T AddScript<T>() where T : Entity, new()
        {
            if (HasScript<T>())
                return null;

            var script = new T { Uuid = Uuid };
            InternalCalls.AddScript(Uuid, typeof(T));
            return script;
        }

        /// <summary>
        /// Looks up an entity by name in the engine. Returns null if not found.
        /// </summary>
        public static Entity FindByName(string name)
        {
            var foundUuid = InternalCalls.FindEntityByNameCall(name);
            return foundUuid == 0 ? null : new Entity(foundUuid);
        }

        /// <summary>
        /// Instantiates (clones) another entity.
        /// </summary>
        public static Entity Instantiate(Entity other)
        {
            return new Entity(InternalCalls.Instantiate(other.Uuid));
        }

        /// <summary>
        /// Deletes this entity from the engine, removing it from the scene.
        /// </summary>
        public void Delete()
        {
            InternalCalls.EntityDelete(Uuid);
        }
    }

    /// <summary>
    /// Base for any engine component. Inherits from entity, but typically
    /// also has a reference to the owning Entity. 
    /// </summary>
    public abstract class Component : Entity
    {
        public new ulong Uuid; // Deliberately shadowing the entity's Uuid
        public Entity Entity { get; internal set; }

        /// <summary>
        /// Whether this component is enabled.
        /// </summary>
        public bool Enabled
        {
            get => InternalCalls.Component_IsEnabled(Uuid, GetType());
            set => InternalCalls.Component_SetEnabled(Uuid, GetType(), value);
        }
    }

    /// <summary>
    /// Represents a transformation component for an Entity (position, rotation, scale).
    /// </summary>
    public class Transform : Component
    {
        public Vector3 Translation
        {
            get
            {
                InternalCalls.GetPositionCall(Entity.Uuid, out var translation);
                return translation;
            }
            set => InternalCalls.SetPosition(Uuid, ref value);
        }

        public Quaternion Rotation
        {
            get
            {
                InternalCalls.GetRotationQuaternionCall(Entity.Uuid, out var quat);
                return new Quaternion(quat.X, quat.Y, quat.Z, quat.W);
            }
            set
            {
                var q = new Vector4(value.x, value.y, value.z, value.w);
                InternalCalls.SetRotationQuaternion(Uuid, ref q);
            }
        }

        public Vector3 Scale
        {
            get
            {
                InternalCalls.GetScaleCall(Entity.Uuid, out var scale);
                return scale;
            }
            set => InternalCalls.SetScaleCall(Uuid, ref value);
        }

        public Vector3 ForwardVector
        {
            get
            {
                InternalCalls.Transform_GetForwardVector(Entity.Uuid, out var vec);
                return vec;
            }
        }

        public Vector3 UpVector
        {
            get
            {
                InternalCalls.Transform_GetUpVector(Entity.Uuid, out var vec);
                return vec;
            }
        }

        public Vector3 LeftVector
        {
            get
            {
                InternalCalls.Transform_GetLeftVector(Entity.Uuid, out var vec);
                return vec;
            }
        }

        public Matrix4 WorldMatrix
        {
            get
            {
                var raw = InternalCalls.Transform_GetWorldMatrix(Entity.Uuid);
                var matrix4x4 = new float[4, 4];
                int idx = 0;

                for (int i = 0; i < 4; i++)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        matrix4x4[i, j] = raw[idx++];
                    }
                }

                return new Matrix4(matrix4x4);
            }
            set
            {
                // Possibly implement if the engine allows setting the full matrix
                // InternalCalls.Transform_SetWorldMatrix(Uuid, ref value);
            }
        }

        public void MoveTowards(Vector3 target)
        {
            InternalCalls.MoveTowards(Uuid, target);
        }

        public Vector3 MoveTowardsWithReturn(Vector3 target)
        {
            InternalCalls.MoveTowardsReturn(Uuid, target, out var outVector);
            return outVector;
        }
    }

    /// <summary>
    /// Encapsulates raw mesh data (vertices, indices, etc.).
    /// </summary>
    public class Mesh
    {
        public ulong uuid;
        public Vector3[] Vertices;
        public int[] Indices;
        public Vector3[] Normals;
        public Vector2[] Uvs;

        /// <summary>
        /// Creates a new mesh with reversed triangle ordering (useful for flipping faces).
        /// </summary>
        public Mesh InvertTriangleOrder()
        {
            var newMesh = this;  // same reference for now
            newMesh.Indices = Indices.Reverse().ToArray();
            return newMesh;
        }
    }

    /// <summary>
    /// Renders a mesh on an entity.
    /// </summary>
    public class MeshRenderer : Component
    {
        public Mesh Mesh
        {
            set => InternalCalls.MeshRenderer_SetMesh(Uuid,
                                                      value.Vertices,
                                                      value.Indices,
                                                      value.Normals,
                                                      value.Uvs);
        }

        public void SetMaterial(ulong matUuid)
        {
            InternalCalls.MeshRenderer_SetMaterial(Uuid, matUuid);
        }

        public Vector3[] Vertices
        {
            get => InternalCalls.MeshRenderer_GetVertices(Entity.Uuid);
            set => InternalCalls.MeshRenderer_SetVertices(Uuid, value);
        }

        public int[] Indices
        {
            get => InternalCalls.MeshRenderer_GetIndices(Entity.Uuid);
            set => InternalCalls.MeshRenderer_SetIndices(Uuid, value);
        }

        public Vector3[] Normals
        {
            get => InternalCalls.MeshRenderer_GetNormals(Entity.Uuid);
            set => InternalCalls.MeshRenderer_SetNormals(Uuid, value);
        }

        public Vector2[] Uvs
        {
            get => InternalCalls.MeshRenderer_GetUvs(Entity.Uuid);
            set => InternalCalls.MeshRenderer_SetUvs(Uuid, value);
        }
    }

    /// <summary>
    /// Stub for an animation component. Implementation depends on your engine.
    /// </summary>
    public class AnimationComponent : Component
    {
    }

    #region RigidBody

    public struct AngularLock
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

    /// <summary>
    /// Physics rigid body component for controlling forces, velocity, locking axes, etc.
    /// </summary>
    public class RigidBody : Component
    {
        public float Drag
        {
            get => InternalCalls.RigidBody_GetDrag(Uuid);
            set => InternalCalls.RigidBody_SetDrag(Uuid, value);
        }

        public Vector3 Velocity
        {
            get
            {
                InternalCalls.RigidBody_GetVelocity(Uuid, out var val);
                return val;
            }
            set => InternalCalls.RigidBody_SetVelocity(Uuid, ref value);
        }

        public void ApplyForce(Vector3 force)
        {
            InternalCalls.RigidBody_ApplyForce(Uuid, ref force);
        }

        public void AddForce(Vector3 force, ForceMode mode = ForceMode.FORCE, bool autoWake = true)
        {
            InternalCalls.RigidBody_AddForce(Uuid, ref force, mode, autoWake);
        }

        public void AddTorque(Vector3 torque, ForceMode mode = ForceMode.FORCE, bool autoWake = true)
        {
            InternalCalls.RigidBody_Add

