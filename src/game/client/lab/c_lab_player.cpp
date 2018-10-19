#include "cbase.h"
#include "c_lab_player.h"

#if defined( CLabPlayer )
	#undef CLabPlayer
#endif


IMPLEMENT_CLIENTCLASS_DT( C_LabPlayer, DT_LabPlayer, CLabPlayer )
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_LabPlayer )
END_PREDICTION_DATA()

C_LabPlayer::C_LabPlayer()
{
}


C_LabPlayer::~C_LabPlayer()
{
}
