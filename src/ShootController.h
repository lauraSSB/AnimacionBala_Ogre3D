#ifndef __ShootController__h__
#define __ShootController__h__

class ShootController
{
public:
    using Self = ShootController;

public: 
    ShootController( ) = default;
    ~ShootController( ) = default;

    void update( double elapsedSeconds );

    float getCursor( ) const;

    float getPower( ) const;

    static float powerToSpeed( float power );

private:
    float m_Cursor      { 0.0f };
    float m_Direction   { 1.0f };
};

#endif