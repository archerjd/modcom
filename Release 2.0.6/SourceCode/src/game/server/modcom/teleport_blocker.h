#ifndef TELEPORTBLOCKER_H
#define TELEPORTBLOCKER_H
#ifdef _WIN32
#pragma once
#endif

#define PREVIOUS_TELEPORT_BLOCKER_RADIUS 1600.0f
#define NEW_TELEPORT_BLOCKER_RADIUS 500.0f

class CTeleportBlocker : public CBaseEntity
{
public:
	DECLARE_CLASS( CTeleportBlocker, CBaseEntity );
	
	CTeleportBlocker();
	void Spawn( void );

	float GetRadius() { return m_flRadius; }
	float GetRadiusSq() { return m_flRadius * m_flRadius; }
	void SetRadius(float radius) { m_flRadius = radius; m_bHasRadius = radius > 0; }
	void SetVisible(bool bVisible);
	
	void VisibleThink();

	DECLARE_DATADESC();
	bool m_bHasRadius;

private:
	float m_flRadius;
	bool m_bVisible;
};

#endif