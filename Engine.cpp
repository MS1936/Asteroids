#include "framework.h"
#include "Ship.h"
#include "Engine.h"

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

Engine::Engine() : m_pDirect2dFactory(NULL), m_pRenderTarget(NULL), m_pWhiteBrush(NULL)
{
    // Initilize the main ship
    ship = new Ship();

    
    noProjectiles = 0;

    // Initializes 6 big asteroids
    noAsteroids = 6;
    for (int i = 0; i < noAsteroids; i++)
    {
        asteroids[i] = new Asteroid();
    }

    // draw lives
    lives = 3;
    for (int i = 0; i < lives; i++)
    {
        Ship* lifeShip = new Ship(i);
        lifeShips[i] = lifeShip;
    }

    // Reset keys
    leftPressed = false;
    rightPressed = false;
    accelerationPressed = false;
    firePressed = 0;

    gameOver = false;
}

Engine::~Engine()
{
    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
}

HRESULT Engine::InitializeD2D(HWND m_hwnd)
{
    // Initializes Direct2D
    D2D1_SIZE_U size = D2D1::SizeU(RESOLUTION_X, RESOLUTION_Y);
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
    m_pDirect2dFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hwnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
        &m_pRenderTarget
    );

    // Initialize text writing factory and format
    DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(m_pDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
    );

    m_pDWriteFactory->CreateTextFormat(
        L"Verdana",
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        60,
        L"", //locale
        &m_pTextFormat
    );

    m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White),
        &m_pWhiteBrush
    );

  //initialize direct2d for ship and other game entities
    ship->InitializeD2D(m_pRenderTarget);

    
    for (int i = 0; i < noAsteroids; i++)
    {
        asteroids[i]->InitializeD2D(m_pRenderTarget);
    }

   
    for (int i = 0; i < lives; i++)
    {
        lifeShips[i]->InitializeD2D(m_pRenderTarget);
    }

    return S_OK;
}

//take user inputs
void Engine::KeyUp(WPARAM wParam)
{
   
    if (!gameOver || gameWon)
    { 
        if (wParam == VK_LEFT)
            leftPressed = false;
        if (wParam == VK_RIGHT)
            rightPressed = false;
        if (wParam == VK_UP)
            accelerationPressed = false;
        if (wParam == VK_SPACE)
            if (firePressed == 2)
                firePressed = 0;
    }
}

void Engine::KeyDown(WPARAM wParam)
{
    
    if (!gameOver || gameWon)
    { 
        if (wParam == VK_LEFT)
            leftPressed = true;
        if (wParam == VK_RIGHT)
            rightPressed = true;
        if (wParam == VK_UP)
            accelerationPressed = true;
        if (wParam == VK_SPACE)
            if (firePressed == 0)
                firePressed = 1;
    }
}

void Engine::Logic(double elapsedTime)
{
   


    if (leftPressed)
    {
        if (!ship->IsExploded())
        {
           
            ship->ApplyLeftRotation(elapsedTime);
        }
    }
    if (rightPressed)
    {
        if (!ship->IsExploded())
        {
           
            ship->ApplyRightRotation(elapsedTime);
        }
    }
    if (accelerationPressed)
    {
        if (!ship->IsExploded())
        {
           
            ship->ApplyAcceleration(elapsedTime);
        }
    }
    if (firePressed == 1)
    {
        if (!ship->IsExploded())
        {
            if (noProjectiles < 20)
            {
                // firing projectiles
                Projectile* projectile = new Projectile(ship->GetPosition(), ship->GetRotation());
                projectile->InitializeD2D(m_pRenderTarget);
                projectiles[noProjectiles] = projectile;
                noProjectiles++;
            }
        }
        firePressed = 2;
    }

    // move ship
    ship->Advance(elapsedTime);
    if (ship->IsExploded() && ship->GetExplosionTime() > 0.5)
    {
        
        ship->Reset();
        lives--;
        if (lives < 0)
        {
            // Game Over
            gameOver = true;
            gameWon = false;
        }
    }

    // Projectile logic
    for (int i = 0; i < noProjectiles; i++)
    {
        // Move the projectiles
        projectiles[i]->Advance(elapsedTime);
        if (projectiles[i]->IsOut())
        {
            // Eliminate the projectile from the list if it's outside the screen
            Projectile* projectile = projectiles[i];
            for (int j = i; j < noProjectiles - 1; j++)
            {
                projectiles[j] = projectiles[j + 1];
            }
            projectiles[noProjectiles - 1] = NULL;
            delete projectile;
            noProjectiles--;
            i--;
        }
    }

    // Asteroid logic
    for (int i = 0; i < noAsteroids; i++)
    {
        // Move the asteroids
        asteroids[i]->Advance(elapsedTime);
        if (asteroids[i]->GetSize() == 0 && asteroids[i]->GetExplosionTime() > 0.5)
        {
            // If the asteroid is exploded and 0.5 seconds passed, remove the asteroid from the list
            Asteroid* asteroid = asteroids[i];
            for (int k = i; k < noAsteroids - 1; k++)
            {
                asteroids[k] = asteroids[k + 1];
            }
            asteroids[noAsteroids - 1] = NULL;
            delete asteroid;
            noAsteroids--;

            if (noAsteroids == 0)
            {
                // You won!
                gameOver = true;
                gameWon = true;
            }
        }
    }

    // Projectile to asteroid collisions
    bool foundCollision = false;
    
    for (int i = 0; i < noAsteroids; i++)
    {
        for (int j = 0; j < noProjectiles; j++)
        {
            if (!foundCollision) 
            {
                Projectile* projectile = projectiles[j];
                Asteroid* asteroid = asteroids[i];

                // Distance between the center of the asteroid and projectile
                double distance = pow(asteroid->GetPosition().x - projectile->GetPosition().x, 2) + pow(asteroid->GetPosition().y - projectile->GetPosition().y, 2);
                // Size of the asteroid
                double size = pow(asteroid->GetSize() * ASTEROID_SIZE_MULTIPLIER, 2) * 1.2;

                if (distance < size)
                { 

                    // Eliminate projectile
                    for (int k = j; k < noProjectiles - 1; k++)
                    {
                        projectiles[k] = projectiles[k + 1];
                    }
                    projectiles[noProjectiles - 1] = NULL;
                    delete projectile;
                    noProjectiles--;

                    // Explode asteroid and create 2 new if needed
                    if (asteroid->GetSize() > 1)
                    {
                        
                        Point2D cSpeed = asteroid->GetSpeed();

                        // New asteroid 1
                        Point2D newSpeed1;
                        newSpeed1.x = cSpeed.y * 1.5;
                        newSpeed1.y = cSpeed.x * 1.5;
                        Asteroid* newAsteroid1 = new Asteroid(asteroid->GetPosition(), asteroid->GetSize() / 2, newSpeed1);
                        newAsteroid1->InitializeD2D(m_pRenderTarget);
                        asteroids[noAsteroids] = newAsteroid1;
                        noAsteroids++;

                        // New asteroid 2
                        Point2D newSpeed2;
                        newSpeed2.x = -cSpeed.y * 1.5;
                        newSpeed2.y = -cSpeed.x * 1.5;
                        Asteroid* newAsteroid2 = new Asteroid(asteroid->GetPosition(), asteroid->GetSize() / 2, newSpeed2);
                        newAsteroid2->InitializeD2D(m_pRenderTarget);
                        asteroids[noAsteroids] = newAsteroid2;
                        noAsteroids++;

                        // Remove old asteroid
                        for (int k = i; k < noAsteroids - 1; k++)
                        {
                            asteroids[k] = asteroids[k + 1];
                        }
                        asteroids[noAsteroids - 1] = NULL;
                        delete asteroid;
                        noAsteroids--;
                    }
                    else
                    {
                        // If atseroid size was 1, we set it to explosion mode
                        asteroid->Explode();
                    }

                    foundCollision = true;
                }
            }
        }
    }


    // Ship to asteroid collisions
    if (!ship->IsExploded())
    {
        foundCollision = false;
        
        for (int i = 0; i < noAsteroids; i++)
        {
            if (!foundCollision) 
            {
                Asteroid* asteroid = asteroids[i];

                // Distance between ship's and asteroid's centers
                double distance = pow(asteroid->GetPosition().x - ship->GetPosition().x, 2) + pow(asteroid->GetPosition().y - ship->GetPosition().y, 2);
                // Asteroid's size + ship's size
                double size = pow(asteroid->GetSize() * ASTEROID_SIZE_MULTIPLIER + 5, 2);

               
                if (distance < size)
                {
                    // Ship explosion
                    ship->Explode();

                    foundCollision = true;
                }
            }
        }
    }

}

//handle drawing of all entities in game
HRESULT Engine::Draw()
{
    
    HRESULT hr;

    m_pRenderTarget->BeginDraw();

    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());


    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

    // Draw projectile
    for (int i = 0; i < noProjectiles; i++)
    {
        projectiles[i]->Draw(m_pRenderTarget);
    }

    if (!gameOver || gameWon)
    {
        // Draws the ship
        ship->Draw(m_pRenderTarget);
    }

    // Draw asteroids
    for (int i = 0; i < noAsteroids; i++)
    {
        asteroids[i]->Draw(m_pRenderTarget);
    }

    // Draw lives
    for (int i = 0; i < lives; i++)
    {
        lifeShips[i]->Draw(m_pRenderTarget);
    }

    // Game Over/ You win screen
    if (gameOver)
    {
        D2D1_RECT_F rectangle2 = D2D1::RectF(0, 0, RESOLUTION_X, RESOLUTION_X);

        if (gameWon)
        {
            m_pRenderTarget->DrawText(
                L"You Win!",
                8,
                m_pTextFormat,
                rectangle2,
                m_pWhiteBrush
            );
        }
        else
        {
            m_pRenderTarget->DrawText(
                L"Game Over!",
                10,
                m_pTextFormat,
                rectangle2,
                m_pWhiteBrush
            );
        }

    }

    hr = m_pRenderTarget->EndDraw();

    return S_OK;
}