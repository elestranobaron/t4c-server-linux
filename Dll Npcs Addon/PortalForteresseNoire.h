

#ifndef __PORTALFORTERESSENOIRE_H
#define __PORTALFORTERESSENOIRE_H

class PortalForteresseNoire : public NPCstructure{
public:   
  PortalForteresseNoire();
  ~PortalForteresseNoire();
	void Create( void );
	void OnPopup( UNIT_FUNC_PROTOTYPE );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnAttacked( UNIT_FUNC_PROTOTYPE );
	void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif
