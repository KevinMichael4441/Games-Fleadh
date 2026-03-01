#include "security_camera.h"

static bool Cam_CheckCollision_Player( Vector2 t_center, float t_radius, Vector2 t_lineStart, Vector2 t_lineEnd);

SecurityCamera::SecurityCamera()
{ 
}


void SecurityCamera::initialize(float t_x, float t_y, float t_distance, CamType t_type, CamMount t_mount, LaserDir t_dir)
{
    m_position = {t_x, t_y};

    m_texture = LoadTexture("./assets/images/ENVIRONMENT/CAMS.png");

    m_frameWidth = 32;
    m_frameHeight = 32;
    m_scale = 1;

    m_frameCount = 9;
    m_frameTime = 0.12f;
    m_currentFrame = 0;
    m_animationTimer = 0.0f;

    m_currentFrame = 0;
    m_targetFrame = 0;
    m_animating = false;
    m_previousActive = true;

    if(MOUNT_BACKGROUND == t_mount)
    {
        m_sourceY = (float)m_frameHeight;
    }
    else
    {
        m_sourceY = 0.0f;
    }

    // This now takes new data from tiled
    // t_distance is still used for m_length to set the length of the laser beam
    // CamType is used to determine the behavior (line that moves side to side on a fixed axis versys line that seeps on a radius) of the laser. 1 = CAM_SPOT, 2 = CAM_SWEEP
    // CamMount is used to determine what texture the camera should use and also the rotation of the camera
    // MOUNT_BACKGROUND uses the front facing texture and the other three types us the side facing texture
    // MOUNT_LEFT_WALL, MOUNT_RIGHT_WALL, and MOUNT_CEILING can also be used to determine what way the side facing texture should be rotated (not sure what the default rotation is)
    // LaserDir is used to determine which direction from the camera the laser should be pointing

    m_type = t_type;
    //m_fix = t_direction; //not sure what you were using fix for but t_direction has been replaced with either t_dir or t_mount depending on if you used it for camera direction or laser direction
    m_origin = { t_x + WIDTH / 2.0f, t_y + HEIGHT / 2.0f };
    m_length = t_distance;
	m_activeDuration   = 5.0f;
	m_inactiveDuration = 5.0f;
	m_timer            = 0.0f;
	m_isActive         = true;


    //this needs to be changed *****************************************
    /*switch(m_fix){
        case N:
            m_angle = 3.125f;
            MAX_ANGLE = 3.85f;
	        MIN_ANGLE = 2.35f;
        break;
        case S:
            m_angle = 0.625f;
            MAX_ANGLE = 0.75f;
	        MIN_ANGLE = -0.75f;
        break;
        case E:
            m_angle = 1.875f;
            MAX_ANGLE = 2.40f;
	        MIN_ANGLE = 0.825f;
        break;
        case W:
            m_angle = -1.875f;
            MAX_ANGLE = -0.825f;
	        MIN_ANGLE = -2.40f;
        break;
        case NE:
            m_angle = 2.5f;
            MAX_ANGLE = 3.125f;
	        MIN_ANGLE = 1.575f;
        break;
        case NW:
            m_angle = 3.75f;
            MAX_ANGLE = 4.725f;
	        MIN_ANGLE = 3.125f;
        break;
        case SE:
            m_angle = 0.75f;
            MAX_ANGLE = 1.5f;
	        MIN_ANGLE = 0.0f;
        break;
        case SW: // Good
            m_angle = -0.75f;
            MAX_ANGLE = 0.0f;
	        MIN_ANGLE = -1.575f;
        break;
    }*/

    switch(m_type){
        case CAM_SPOT:
            m_end = (Vector2){sin(m_angle) * m_length + m_origin.x, cos(m_angle) * m_length + m_origin.y};
        break;
        case CAM_SWEEP:
            m_end = (Vector2){m_origin.x, m_origin.y + m_length};
        break;
    }
    
    m_direction = Vector2Normalize((Vector2){m_end.x - m_origin.x, m_end.y - m_origin.y});

	m_body = {t_x, t_y, WIDTH, HEIGHT}; // Temp

    m_isActive = true;
	m_playerDetected = false;
    m_movingRight = true;

    m_laser.p = c2V(m_origin.x, m_origin.y);// Starting point
    m_laser.d = c2V(m_direction.x , m_direction.y); // Direction
    m_laser.t = m_length; // Length

    angleV = 0.015f;
}

bool SecurityCamera::raycastPlayerCollision(Vector2& t_center){
    c2Circle player;
    c2Raycast laserCast;
    player.p = c2V(t_center.x, t_center.y);
    player.r = 30.0f;

    return c2RaytoCircle(m_laser, player, &laserCast);
}
void SecurityCamera::drawRaycast()
{
    float r = 2.0f;
	DrawLineEx((Vector2){m_laser.p.x, m_laser.p.y}, m_end, r, RED);
}

void SecurityCamera::update(float t_dt, Vector2 playerPos)
{
    m_timer += t_dt;

    if (m_isActive)
    {
        if (m_timer >= m_activeDuration)
        {
            m_isActive = false;
            m_timer = 0.0f;
        }
    }
    else
    {
        if (m_timer >= m_inactiveDuration)
        {
            m_isActive = true;
            m_timer = 0.0f;
            }
    }

    if (m_isActive != m_previousActive)
    {
        m_animating = true;

        if (m_isActive)
            m_targetFrame = m_frameCount - 1;
        else
            m_targetFrame = 0;

        m_previousActive = m_isActive;
    }

    Frame_Update(t_dt);
	
    if (!m_isActive) return;
    
    if(m_angle >= MAX_ANGLE && angleV > 0.0f){
        angleV *= -1;
        m_movingRight = false;
    }
    if(m_angle <= MIN_ANGLE && angleV <= 0.0f){
        angleV *= -1;
        m_movingRight = true;
    }
    m_angle += angleV; 
    
    switch(m_type){
        case CAM_SWEEP:
            if(m_movingRight == true){m_end.x += extendSpd;}
            else{m_end.x -= extendSpd;}
        break;
        case CAM_SPOT:
            m_end = (Vector2){sin(m_angle) * m_length + m_origin.x, cos(m_angle) * m_length + m_origin.y};
        break;
        default:
        break;
    }

    m_direction = Vector2Normalize((Vector2){m_end.x - m_origin.x, m_end.y - m_origin.y});
    m_laser.d = c2V(m_direction.x , m_direction.y);
    m_playerDetected = raycastPlayerCollision(playerPos);
}

void SecurityCamera::SetLevel(LevelData* level)
{
    m_level = level;
}

bool SecurityCamera::isPlayerDetected() const
{
    return m_playerDetected;
}

void SecurityCamera::draw()
{
    Animate();

    if (!m_isActive) return;

    drawRaycast();

    if (m_playerDetected)
    {
        printf("Player Detected\n");
        DrawCircleV(m_origin, 6, GREEN);
    }
}

void SecurityCamera::Frame_Update(float dt)
{
    if (!m_animating) return;

    m_animationTimer += dt;

    if (m_animationTimer >= m_frameTime)
    {
        m_animationTimer = 0.0f;

        if (m_currentFrame < m_targetFrame)
        {
            m_currentFrame++;
        }
        else if (m_currentFrame > m_targetFrame)
        {
            m_currentFrame--;
        }

        if (m_currentFrame == m_targetFrame)
        {
            m_animating = false;
        }
    }
}

void SecurityCamera::Animate() 
{
    Rectangle source = { (float)(m_currentFrame * m_frameWidth), m_sourceY, (float)m_frameWidth, (float)m_frameHeight };
    DrawTextureRec(m_texture, source, m_position, WHITE);
}