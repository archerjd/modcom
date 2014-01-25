#ifndef LIMITED_H
#define LIMITED_H
#pragma once

#include "modcom/mcconvar.h"

enum limited_quantity_t
{
	LQ_MINION=0,
	LQ_LASER,
	LQ_CROW,
//	LQ_BARNACLE,
	LQ_MAGMINE,
//	LQ_REPMINE,
	LQ_TURRET,
	LQ_MANHACK,
};


class CHL2MP_Player;
typedef void (*ClearFunc)(CHL2MP_Player *);

class LimitedQuantity
{
public:
	LimitedQuantity(limited_quantity_t name, McConVar *limit_base, McConVar *limit_scale, McConVar *limit_power, int relatedModuleIndex, ClearFunc f)
	{
		m_name = name;
		m_limit_base = limit_base;
		m_limit_scale = limit_scale;
		m_limit_power = limit_power;
		m_count = 0;
		m_ModuleIndex = relatedModuleIndex;
		m_clearFunc = f;
	}

	limited_quantity_t GetName() { return m_name; }
	int GetLimit(CHL2MP_Player *pPlayer);
	int GetCount() { return m_count; }
	bool IsFull(CHL2MP_Player *pPlayer, int desiredNumToAdd=1) { /*if ( GetCount() >= GetLimit() ) Msg("%s is full\n",GetName()); else Msg("%s is not full\n",GetName());*/ return GetCount() + desiredNumToAdd > GetLimit(pPlayer); }
	void Add(int num=1) { m_count += num; /*Msg("%s count is now %i\n",GetName(),GetCount());*/ }
	void Remove(int num=1) { m_count = max(m_count-num,0); /*Msg("%s count is now %i\n",GetName(),GetCount());*/ }
	void SetCountToZero() { m_count = 0; /*Msg("%s count is now %i\n",GetName(),GetCount());*/ } // does NOT call the ClearFunc!
	ClearFunc GetClearFunction() { return m_clearFunc; }

	bool operator==( LimitedQuantity *val ) const;

private:
	limited_quantity_t m_name;
	int m_count;
	int m_ModuleIndex;
	ClearFunc m_clearFunc;
	McConVar *m_limit_base, *m_limit_scale, *m_limit_power;
};

class LimitedQuantities
{
public:
	LimitedQuantities(CHL2MP_Player *pPlayer);
	~LimitedQuantities();

	virtual int GetLimit(limited_quantity_t name);
	virtual int GetCount(limited_quantity_t name);
	virtual bool IsFull(limited_quantity_t name,int desiredNumToAdd=1);
	virtual void Add(limited_quantity_t name,int num=1);
	virtual void Remove(limited_quantity_t name,int num=1);

	virtual void AddNewType(limited_quantity_t name, McConVar *limit_base, McConVar *limit_scale, McConVar *limit_power, int moduleIndex, ClearFunc c);
	virtual void ResetAll();
	virtual void Reset(limited_quantity_t name);

private:
	LimitedQuantity *GetByName(limited_quantity_t name);
	CHL2MP_Player *m_pMyPlayer;
	CUtlVector<LimitedQuantity*> m_quantities;
};

inline bool LimitedQuantity::operator==( LimitedQuantity *val ) const
{
	return m_name == val->GetName();
}

#endif