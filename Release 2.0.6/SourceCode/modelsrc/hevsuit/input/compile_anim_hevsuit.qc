$modelname Humans\HEV_Body.mdl
$cdmaterials models\HEVSuit\

$texturegroup hevsuit_sheet
{
	{ "hevsuit_sheet"  }
	{ "hevsuit_sheet_ap" }
}

$texturegroup hevsuit_sheet_emblem
{
	{ "hevsuit_sheet_emblem"  }
	{ "hevsuit_sheet_emblem_ap" }
}

// head controllers
$attachment "eyes" "ValveBiped.Bip01_Head1" -0.0454 -3.2682 67.3843 absolute
$attachment "mouth" "ValveBiped.Bip01_Head1" 0.80 -5.50 0.10 rotate 0 -80 -90
$attachment "chest" "ValveBiped.Bip01_Spine2" 5.00 4.00 0.00 rotate 0 90 90

$model hev_body "hevsuit_rigged.smd"

$surfaceprop "flesh"
$bbox -13 -13 0  13 13 72
$mostlyopaque

$include "standardhierarchy.qci"
$include "standardikchains.qci"
$alwayscollapse "Male_01"
$alwayscollapse "XSISceneRoot"
$proceduralbones "male.vrd"
$include "commonbones.qci"

$includemodel humans/male_shared.mdl
$includemodel humans/male_ss.mdl
$includemodel humans/male_gestures.mdl
$includemodel humans/male_postures.mdl


$pushd "C:\DevDocs\games\modularcombat\SourceCode\modelsrc\hevsuit\input\"
$include "C:\DevDocs\games\modularcombat\SourceCode\modelsrc\hevsuit\input\hitbox.qci"
$include "C:\DevDocs\games\modularcombat\SourceCode\modelsrc\hevsuit\input\ragdoll.qci"
$popd