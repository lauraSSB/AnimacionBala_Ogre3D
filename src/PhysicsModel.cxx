#include "PhysicsModel.h"

#include <stdexcept>
#include <Ogre.h>

//--------------------------------------------------------------------------
//Crea el mundo de física y le agrega un cuerpo rígido a cada entidad que tenga collider. 
void PhysicsModel::
build( Ogre::SceneNode* root, const Ogre::Vector3& gravity)
{
    this->m_World = std::make_unique< Ogre::Bullet::DynamicsWorld >( gravity );
    this->_buildNode( root );
    this->m_Simulating = false;
}

//--------------------------------------------------------------------------
// Convierte una entidad de Ogre en un cuerpo rígido de Bullet con las propiedades
btRigidBody* PhysicsModel::
addBody( Ogre::Entity* entity, const BodyProperties& props )
{
    if( this->m_World == nullptr )
        throw std::runtime_error( "PhysicsModel: Call build() first" );
    if( entity == nullptr )
        throw std::runtime_error( "PhysicsModel: null entity" );

    btRigidBody* body
        = this->m_World->addRigidBody( props.mass, entity, props.collider );
    body->setRestitution( props.restitution );
    body->setFriction( props.friction );
    body->setRollingFriction( props.rollingFriction );
    body->setDamping( props.linearDamping, props.angularDamping );

    if( props.mass > 0 ){
        const float scale = entity->getParentSceneNode( )->_getDerivedScale( ).x;
        const float radius = entity->getBoundingRadius( ) * scale;
        body->setCcdMotionThreshold( radius );
        body->setCcdSweptSphereRadius( radius * 0.8f );
    }
    
    return( body );
}

//--------------------------------------------------------------------------
//Encender o apagar la simulación. CUando esta apagada los objetos quedan congelados
void PhysicsModel::
setSimulating( bool simulating )
{
    this->m_Simulating = simulating;
}

//--------------------------------------------------------------------------
//Indica si la simulación está  corriendo
bool PhysicsModel::
isSimulating( ) const
{
    return( this->m_Simulating );
}

//--------------------------------------------------------------------------
//Actualiza la simulación el tiempo transcurrido desde el frame anterior
void PhysicsModel::
update( double elapsedSeconds )
{
    if( this->m_Simulating && this->m_World != nullptr )
        this->m_World->getBtWorld( )->stepSimulation( elapsedSeconds, 10 );
}

//--------------------------------------------------------------------------
//Rercorre el arbol de ogre y asigna un cuerpo rigido. Se ignoran cámaras, luces y decoración
void PhysicsModel::
_buildNode( Ogre::SceneNode* node )
{
    for( Ogre::MovableObject* object : node->getAttachedObjects( ))
    {
        auto* entity = dynamic_cast< Ogre::Entity* >( object );
        if( entity == nullptr )
            continue;

        BodyProperties props;
        if( Self::_readBodyProperties( entity->getUserObjectBindings( ), props ) )
            this->addBody( entity, props );
    }

    for( Ogre::Node* child : node->getChildren( ) )
        this->_buildNode( static_cast< Ogre::SceneNode* >( child ) );
}

//--------------------------------------------------------------------------
//Lee las propiedades del user data y las guarda en props
bool PhysicsModel::
_readBodyProperties(
    const Ogre::UserObjectBindings& userData, BodyProperties& props
)
{
    const Ogre::Any& collider = userData.getUserAny( "collider" );
    if( !collider.has_value( ) )
        return( false );

    props.collider          = Self::_parseCollider( Ogre::any_cast< Ogre::String >( collider ) );
    props.mass              = Self::_readFloat( userData, "mass",             props.mass );
    props.restitution       = Self::_readFloat( userData, "restitution",      props.restitution );
    props.friction          = Self::_readFloat( userData, "friction",         props.friction );
    props.rollingFriction   = Self::_readFloat( userData, "rolling_friction", props.rollingFriction );
    props.linearDamping     = Self::_readFloat( userData, "linear_damping",   props.linearDamping );
    props.angularDamping    = Self::_readFloat( userData, "angular_damping",  props.angularDamping );
    return( true );
}

//--------------------------------------------------------------------------
//Si alguna propiedad no existe, devuelve el valor por defecto
float PhysicsModel::
_readFloat(
    const Ogre::UserObjectBindings& userData,
    const std::string& key,
    float defaultValue
    )
{
    const Ogre::Any& value = userData.getUserAny( key );
    return( value.has_value( ) ? Ogre::any_cast< float >( value ) : defaultValue );
}

//--------------------------------------------------------------------------
//Se define el tipo de collider que se usa. 
Ogre::Bullet::ColliderType PhysicsModel::
_parseCollider( const std::string& name )
{
    Ogre::String lower = name;
    Ogre::StringUtil::toLowerCase( lower );

    if( lower == "box"      ) return( Ogre::Bullet::CT_BOX );
    if( lower == "capsule"  ) return( Ogre::Bullet::CT_CAPSULE );
    if( lower == "compound" ) return( Ogre::Bullet::CT_COMPOUND );
    if( lower == "cylinder" ) return( Ogre::Bullet::CT_CYLINDER );
    if( lower == "hull"     ) return( Ogre::Bullet::CT_HULL );
    if( lower == "sphere"   ) return( Ogre::Bullet::CT_SPHERE );
    if( lower == "trimesh"  ) return( Ogre::Bullet::CT_TRIMESH );
    throw std::runtime_error( "PhysicsModel: unknown collider '" + name + "'" );
}