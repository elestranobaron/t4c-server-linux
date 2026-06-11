

#ifndef __PORTALCAVEDESTIN_H
#define __PORTALCAVEDESTIN_H

class PortalCaveDestin : public NPCstructure{
public:   
  PortalCaveDestin();
  ~PortalCaveDestin();
	void Create( void );
	void OnPopup( UNIT_FUNC_PROTOTYPE );
	void OnTalk( UNIT_FUNC_PROTOTYPE );
	void OnAttacked( UNIT_FUNC_PROTOTYPE );
	void OnInitialise( UNIT_FUNC_PROTOTYPE );
};

#endif
