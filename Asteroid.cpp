#include "framework.h"
#include "Point2D.h"
#include "Engine.h"
#include "Asteroid.h"

Asteroid::Asteroid() : m_pWhiteBrush(NULL)
{
	// Initialize position randomly on screen
	position.x = rand() % RESOLUTION_X;
	position.y = rand() % RESOLUTION_Y;

	// Initialize fixed speed in random direction
	double rotationAngle = rand() % 360;
	speed.x = sin(rotationAngle * PI / 180) * ASTEROID_SPEED;
	speed.y = -cos(rotationAngle * PI / 180) * ASTEROID_SPEED;

	
	size = 4;

	// Initializes random rotation speed
	rotation = 0;
	rotationSpeed = rand() % ASTEROID_MAX_ROTATION - (ASTEROID_MAX_ROTATION / 2);

	// Generates random shape of asteroid
	int variation = ASTEROID_SIZE_VARIATION * size / 4;
	for (int i = 0; i < ASTEROID_CORNERS; i++)
	{
		sizeVariation[i] = rand() % variation - (variation / 2.0);
	}
}

Asteroid::Asteroid(Point2D newPosition, int newSize, Point2D newSpeed) : m_pWhiteBrush(NULL)
{
	// Initializes position, speed and size of asteroid
	position = newPosition;
	speed = newSpeed;
	size = newSize;

	// Initializes a random rotation speed
	rotation = 0;
	rotationSpeed = rand() % ASTEROID_MAX_ROTATION - (ASTEROID_MAX_ROTATION / 2);

	// Generates random shape of asteroid
	int variation = ASTEROID_SIZE_VARIATION * size / 4;
	for (int i = 0; i < ASTEROID_CORNERS; i++)
	{
		sizeVariation[i] = rand() % variation - (variation / 2.0);
	}
}

Asteroid::~Asteroid()
{
	SafeRelease(&m_pWhiteBrush);
	
}

void Asteroid::InitializeD2D(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	
	m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White),
		&m_pWhiteBrush
	);
	
	m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White),
		&m_pWhiteBrush
	);
}

void Asteroid::Advance(double elapsedTime)
{
	// collision
	if (size > 0)
	{
		// screen wrapping
		position.x += elapsedTime * speed.x;
		if (position.x < -10)
		{
			position.x = RESOLUTION_X + 10;
		}
		if (position.x > RESOLUTION_X + 10)
		{
			position.x = -10;
		}
		position.y += elapsedTime * speed.y;
		if (position.y < -10)
		{
			position.y = RESOLUTION_Y + 10;
		}
		if (position.y > RESOLUTION_Y + 10)
		{
			position.y = -10;
		}
		// asteroid roation speed timer
		rotation += rotationSpeed * elapsedTime;
	}
	else
	{
		// explosion animation timer
		explosionTime += elapsedTime;
	}
}

void Asteroid::Explode()
{
	// Asteroids goes into explosion mode
	size = 0;
	explosionTime = 0;
}

void Asteroid::Draw(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	if (size > 0)
	{
		// If not exploded, draw the asteroid's shape
		ID2D1PathGeometry* clPath;
		ID2D1Factory* factory;
		m_pRenderTarget->GetFactory(&factory);
		factory->CreatePathGeometry(&clPath);

		ID2D1GeometrySink* pclSink;
		clPath->Open(&pclSink);
		pclSink->SetFillMode(D2D1_FILL_MODE_WINDING);
		D2D1_POINT_2F point0 = D2D1::Point2F(position.x + (size * ASTEROID_SIZE_MULTIPLIER + sizeVariation[0]) * sin(rotation * PI / 180), position.y - (size * ASTEROID_SIZE_MULTIPLIER + sizeVariation[0]) * cos(rotation * PI / 180));
		pclSink->BeginFigure(point0, D2D1_FIGURE_BEGIN_FILLED);
		int angleStep = 360 / ASTEROID_CORNERS;
		for (int i = 1; i < ASTEROID_CORNERS; i++)
		{
			D2D1_POINT_2F point = D2D1::Point2F(position.x + (size * ASTEROID_SIZE_MULTIPLIER + sizeVariation[i]) * sin((rotation + i * angleStep) * PI / 180), position.y - (size * ASTEROID_SIZE_MULTIPLIER + sizeVariation[i]) * cos((rotation + i * angleStep) * PI / 180));
			pclSink->AddLine(point);
		}
		pclSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		pclSink->Close();
		m_pRenderTarget->DrawGeometry(clPath, m_pWhiteBrush, 4);

		SafeRelease(&pclSink);
		SafeRelease(&clPath);

	}
	else
	{
		// draw explosion
		int angleStep = 360 / ASTEROID_CORNERS;
		for (int i = 0; i < ASTEROID_CORNERS; i++)
		{
			D2D1_ELLIPSE ellipseBall = D2D1::Ellipse(
				D2D1::Point2F(position.x + (explosionTime * (100 + 20 * sizeVariation[i])) * sin(i * angleStep * PI / 180), position.y - (explosionTime * (100 + 20 * sizeVariation[i])) * cos(i * angleStep * PI / 180)),
				4, 4
			);
			m_pRenderTarget->FillEllipse(&ellipseBall, m_pWhiteBrush);
		}
	}
}

Point2D Asteroid::GetPosition()
{
	return position;
}

Point2D Asteroid::GetSpeed()
{
	return speed;
}

int Asteroid::GetSize()
{
	return size;
}

double Asteroid::GetExplosionTime()
{
	return explosionTime;
}