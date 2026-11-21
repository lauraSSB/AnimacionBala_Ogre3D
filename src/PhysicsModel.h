#ifndef __PhysicsModel__h__
#define __PhysicsModel__h__

#include <memory>
#include <string>
#include <OgreBullet.h>

namespace Ogre { class Entity; class SceneNode; class UserObjectBindings; }

// Acá se definen las propiedades físicas de un objeto. En este caso se llenan desde el userData del .scene
struct BodyProperties
{
    Ogre::Bullet::ColliderType collider { Ogre::Bullet::CT_BOX };
    float mass              { 0.0f };
    float restitution       { 0.5f };
    float friction          { 0.5f };
    float rollingFriction   { 0.02f };
    float linearDamping     { 0.05f };
    float angularDamping    { 0.1f };
};

// Bullet asociado a una escena de Ogre
class PhysicsModel
{
public: 
    using Self = PhysicsModel;

public: 
    PhysicsModel( ) = default;
    ~PhysicsModel( ) = default; 

    //Crear el mundo y agregar un cuerpo por cada entidad con userData collider
    void build( Ogre::SceneNode* root, const Ogre::Vector3& gravity );

    //Agregar un cuerpo a una entidad
    btRigidBody* addBody( Ogre::Entity* entity, const BodyProperties& props );

    void setSimulating( bool simulating );
    bool isSimulating( ) const; 

    // Actualiza la simulación, se llama una vez por frame
    void update( double elapsedSeconds );

private: 
    void _buildNode( Ogre::SceneNode* node );
    static bool _readBodyProperties( 
        const Ogre::UserObjectBindings& userData, BodyProperties& props
    );
    static float _readFloat(
        const Ogre::UserObjectBindings& userData,
        const std::string& key, 
        float defaultValue
    );
    static Ogre::Bullet::ColliderType _parseCollider( const std::string& name );

private:
    std::unique_ptr< Ogre::Bullet::DynamicsWorld > m_World;
    bool m_Simulating { false };
};

#endif