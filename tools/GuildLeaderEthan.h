/******************************************************************************
Modify for vs2008 (23/04/2009)
******************************************************************************/


#ifndef __GUILDLEADERETHAN_H
#define __GUILDLEADERETHAN_H

class GuildLeaderEthan : public NPCstructure{
public:
	GuildLeaderEthan();
	~GuildLeaderEthan();
	void Create( void );
	void OnTalk( UNIT_FUNC_PROTOTYPE );

};

#endif