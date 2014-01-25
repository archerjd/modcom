#ifndef MCCONVAR_H
#define MCCONVAR_H

#include "tier1/convar.h"

class McConVar : public ConVar
{
public:
	McConVar( const char *pName, const char *pDefaultValue, int flags = 0)
		: ConVar(pName,pDefaultValue,flags)
	{
#ifndef CLIENT_DLL
		AddMeToList();
#endif
	}

	McConVar( const char *pName, const char *pDefaultValue, int flags, char *pHelpString )
		: ConVar( pName, pDefaultValue, flags, pHelpString )
	{
#ifndef CLIENT_DLL
		AddMeToList();
#endif
	}

	McConVar( const char *pName, const char *pDefaultValue, int flags, char *pHelpString, bool bMin, float fMin, bool bMax, float fMax )
		: ConVar( pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax )
	{
#ifndef CLIENT_DLL
		AddMeToList();
#endif
	}

	McConVar( const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, FnChangeCallback_t callback )
		: ConVar( pName, pDefaultValue, flags, pHelpString, callback )
	{
#ifndef CLIENT_DLL
		AddMeToList();
#endif
	}

	McConVar( const char *pName, const char *pDefaultValue, int flags, const char *pHelpString, bool bMin, float fMin, bool bMax, float fMax, FnChangeCallback_t callback )
		: ConVar( pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax, callback )
	{
#ifndef CLIENT_DLL
		AddMeToList();
#endif
	}

#ifndef CLIENT_DLL
	~McConVar() { RemoveMeFromList(); }

	virtual void SetValue( const char *value )	{ ConVar::SetValue(value); OnChanged(); }
	virtual void SetValue( float value )		{ ConVar::SetValue(value); OnChanged(); }
	virtual void SetValue( int value )			{ ConVar::SetValue(value); OnChanged(); }
#endif
	virtual const char*			GetHelpText( void ) const;
	static bool AreAllDefault();
#ifndef CLIENT_DLL
	bool IsDefault() { return FStrEq( GetString(), GetDefault() ); }
	static void CheckAllDefault();
	static void SetAllDefault();

protected:
	void OnChanged();
	void AddMeToList();
	void RemoveMeFromList();
#endif
};

#endif // MCCONVAR_H