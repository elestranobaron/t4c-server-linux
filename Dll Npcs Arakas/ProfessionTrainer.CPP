#pragma hdrstop
#include "ProfessionTrainer.H"

ProfessionTrainer::ProfessionTrainer()
{}

ProfessionTrainer::~ProfessionTrainer()
{}

extern NPCstructure::NPC ProfessionTrainerNPC;

void ProfessionTrainer::Create( void )
{
	npc = ( ProfessionTrainerNPC );
	SET_NPC_NAME( "Profession Trainer" );  
	npc.InitialPos.X = 2500; // Brigand Camp
	npc.InitialPos.Y = 2500;
	npc.InitialPos.world = 0;
}


void ProfessionTrainer::OnInitialise( UNIT_FUNC_PROTOTYPE ){
	NPCstructure::OnInitialise( UNIT_FUNC_PARAM );
	WorldPos wlPos = { 0,0,0 };
	self->SetDestination( wlPos );
	self->Do( nothing );
	self->SetCanMove( FALSE );
}

void ProfessionTrainer::OnTalk( UNIT_FUNC_PROTOTYPE )

{

InitTalk

Begin
INTL( 935, "Yo !!!")

Command2(INTL( 495, "LEARN"),INTL( 496, "TEACH"))
INTL( 948, "Let's stop wasting time then.")
CreateFormuleList
	AddTeachFormule( 1000, 1000)
   AddTeachFormule( 1001, 2000)
   AddTeachFormule( 1002, 3000)
SendTeachFormuleList



Command5(INTL( 518, "BYE"),INTL( 519, "LEAVE"),INTL( 517, "ADIEU"),INTL( 520, "QUIT"),INTL( 521, "EXIT"))
INTL( 961, "Slut !!!") BREAK

Default
INTL( 962, "Pas de temps a perdre avec ca....")

EndTalk
}

//////////////////////////////////////////////////////////////////////////////////////////
void ProfessionTrainer::OnNPCDataExchange( UNIT_FUNC_PROTOTYPE )
//////////////////////////////////////////////////////////////////////////////////////////
// ProfessionTrainer data function
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
	
NPC_DATA_EXCHANGE

	NPC_DATAF_TEACH
		NPC_TEACH_FORMULE_LIST
         TEACH_PROFESSION_FORMULE( 1000, 1000 )
         TEACH_PROFESSION_FORMULE( 1001, 2000 )
         TEACH_PROFESSION_FORMULE( 1002, 3000 )
		CLOSE_NPC_TEACH_FORMULE_LIST
	CLOSE_NPC_DATA_TEACH


CLOSE_NPC_DATA_EXCHANGE
}