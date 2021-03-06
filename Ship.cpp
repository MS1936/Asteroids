#include "framework.h"
#include "Point2D.h"
#include "Engine.h"
#include "Ship.h"

Ship::Ship() : m_pWhiteBrush(NULL)
{
	// Resets ship in the center of the screen
	Reset();
}

Ship::Ship(int lifeNo) : m_pWhiteBrush(NULL)
{
	//sets life counter

	Reset();

	// Sets position in the corner of the screen
	position.x = 30 + lifeNo * 30;
	position.y = 40;
}

Ship::~Ship()
{
	SafeRelease(&m_pWhiteBrush);
}

void Ship::Reset()
{
	// Position in the center of the screen
	position.x = RESOLUTION_X / 2;
	position.y = RESOLUTION_Y / 2;

	
	speed.x = 0;
	speed.y = 0;

	rotation = 0;

	
	exploded = false;
	explosionTime = 0;
}

void Ship::InitializeD2D(ID2D1HwndRenderTarget* m_pRenderTarget)
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

void Ship::ApplyLeftRotation(double elapsedTime)
{
	// Rotate ship left
	rotation -= elapsedTime * 180;
}

void Ship::ApplyRightRotation(double elapsedTime)
{
	// Rotate ship right
	rotation += elapsedTime * 180;
}

void Ship::ApplyAcceleration(double elapsedTime)
{
	// This accellerates the ship forward. We also cap the speed
	speed.x += 250 * elapsedTime * sin(rotation * PI / 180);
	if (speed.x > 100)
	{
		speed.x = 100;
	}
	if (speed.x < -100)
	{
		speed.x = -100;
	}
	speed.y -= 250 * elapsedTime * cos(rotation * PI / 180);
	if (speed.y > 100)
	{
		speed.y = 100;
	}
	if (speed.y < -100)
	{
		speed.y = -100;
	}
}

void Ship::Advance(double elapsedTime)
{
	// Ship moves according to its speed, and if it goes outside the screen, we pop up on the other side of the screen
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
	if (exploded)
	{
		explosionTime += elapsedTime;
	}
}

void Ship::Explode()
{
	// Ship explodes
	exploded = true;
	explosionTime = 0;
}

void Ship::Draw(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	if (!exploded)
	{
		
		D2D1_POINT_2F headPoint = D2D1::Point2F(position.x + 30 * sin(rotation * PI / 180), position.y - 30 * cos(rotation * PI / 180));
		D2D1_POINT_2F leftPoint = D2D1::Point2F(position.x + 15 * sin((rotation - 120) * PI / 180), position.y - 15 * cos((rotation - 120) * PI / 180));
		D2D1_POINT_2F rightPoint = D2D1::Point2F(position.x + 15 * sin((rotation + 120) * PI / 180), position.y - 15 * cos((rotation + 120) * PI / 180));

		ID2D1PathGeometry* clPath;
		ID2D1Factory* factory;
		m_pRenderTarget->GetFactory(&factory);
		factory->CreatePathGeometry(&clPath);

		ID2D1GeometrySink* pclSink;
		clPath->Open(&pclSink);
		pclSink->SetFillMode(D2D1_FILL_MODE_WINDING);
		pclSink->BeginFigure(headPoint, D2D1_FIGURE_BEGIN_FILLED);
		pclSink->AddLine(leftPoint);
		pclSink->AddLine(rightPoint);
		pclSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		pclSink->Close();
		m_pRenderTarget->FillGeometry(clPath, m_pWhiteBrush);

		SafeRelease(&pclSink);
		SafeRelease(&clPath);
	}
	else
	{
	
		int angleStep = 360 / ASTEROID_CORNERS;
		for (int i = 0; i < ASTEROID_CORNERS; i++)
		{
			D2D1_ELLIPSE ellipseBall = D2D1::Ellipse(
				D2D1::Point2F(position.x + (explosionTime * 120) * sin(i * angleStep * PI / 180), position.y - (explosionTime * 120) * cos(i * angleStep * PI / 180)),
				4, 4
			);
			m_pRenderTarget->FillEllipse(&ellipseBall, m_pWhiteBrush);
		}
	}
}

Point2D Ship::GetPosition()
{
	return position;
}

double Ship::GetRotation()
{
	return rotation;
}

bool Ship::IsExploded()
{
	return exploded;
}

double Ship::GetExplosionTime()
{
	return explosionTime;
}