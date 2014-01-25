$cd "C:\DevDocs\games\modularcombat\SourceCode\modelsrc\hevzombie\new_input"
$modelname "Zombie/hev_zomb.mdl"
$model "studio" "Zombie_Soldier_ref.smd"

$model "headcrab1" "UnknownModelName.smd"

$lod 35
 {
  replacemodel "UnknownModelName" "lod3_UnknownModelName"
}
$lod 60
 {
  replacemodel "UnknownModelName" "lod4_UnknownModelName"
}
$lod 90
 {
  replacemodel "UnknownModelName" "lod5_UnknownModelName"
}

$model "headcrab1" "Headcrab_reference.smd"

$lod 35
 {
  replacemodel "Headcrab_reference" "lod3_Headcrab_reference"
}
$lod 60
 {
  replacemodel "Headcrab_reference" "lod4_Headcrab_reference"
}
$lod 90
 {
  replacemodel "Headcrab_reference" "lod5_Headcrab_reference"
}
$shadowlod
{
  replacemodel "Headcrab_reference" "lod6_Headcrab_reference"
}
$cdmaterials "models\zombie_classic\"
$cdmaterials "models\headcrab_classic\"
$cdmaterials "models\combine_soldier\"
$cdmaterials "models\hev_zomb\"
$hboxset "default"
$hbox 2 "ValveBiped.Bip01_Spine1" -8.500  -3.840  -7.120  15.500  9.840  7.730
$hbox 4 "ValveBiped.Bip01_L_UpperArm" -1.000  -3.500  -2.500  11.000  3.500  2.500
$hbox 4 "ValveBiped.Bip01_L_Forearm" -0.500  -1.860  -1.900  10.500  3.110  2.360
$hbox 4 "ValveBiped.Bip01_L_Hand" 0.000  -1.780  -1.000  6.000  1.230  2.000
$hbox 5 "ValveBiped.Bip01_R_UpperArm" -1.000  -3.500  -2.500  11.000  3.500  2.500
$hbox 5 "ValveBiped.Bip01_R_Forearm" -0.500  -1.890  -1.600  10.500  3.090  2.660
$hbox 5 "ValveBiped.Bip01_R_Hand" 0.000  -1.780  -2.500  6.000  1.230  0.500
$hbox 6 "ValveBiped.Bip01_L_Thigh" 0.000  -4.000  -3.730  17.250  4.000  4.290
$hbox 6 "ValveBiped.Bip01_L_Calf" 0.000  -4.870  -3.540  17.250  3.180  2.630
$hbox 6 "ValveBiped.Bip01_L_Foot" -1.500  -2.000  -2.500  7.500  2.000  2.500
$hbox 7 "ValveBiped.Bip01_R_Thigh" 0.000  -4.000  -3.730  17.250  4.000  4.290
$hbox 7 "ValveBiped.Bip01_R_Calf" 0.000  -4.920  -3.120  17.250  3.250  2.650
$hbox 7 "ValveBiped.Bip01_R_Foot" -1.500  -2.000  -2.500  7.500  2.000  2.500
$hbox 1 "ValveBiped.HC_Body_Bone" -6.590  -5.000  -5.760  8.500  5.000  6.240
$attachment "headcrab" "ValveBiped.HC_Rear_Bone" -0.00 -0.00 0.00 rotate -0.00 0.00 -0.00
$attachment "eyes" "ValveBiped.HC_Body_Bone" 0.00 0.00 0.00 rotate -0.00 -0.00 -50.00
$attachment "head" "ValveBiped.HC_Body_Bone" 0.00 0.00 0.00 rotate -0.00 -0.00 -50.00
$attachment "chest" "ValveBiped.Bip01_Spine4" -3.00 6.00 0.00 rotate -0.00 60.00 90.00
$attachment "maw" "ValveBiped.Bip01_Spine2" 0.00 8.00 0.00 rotate -0.00 90.00 90.00
$attachment "Blood_Left" "ValveBiped.Bip01_L_Finger2" 0.00 0.00 0.00 rotate 0.00 -0.00 0.00
$attachment "Blood_Right" "ValveBiped.Bip01_R_Finger2" 0.00 0.00 0.00 rotate 0.00 -0.00 0.00
$includemodel "zombie\zombie_soldier_animations.mdl"
$surfaceprop "zombieflesh"
$eyeposition -0.000 0.000 64.000
$illumposition -1.246 -0.099 35.512
$sequence refpose "refpose" loop fps 30.00
$sequence ragdoll "ragdoll" ACT_DIERAGDOLL 1 fps 30.00
$ikchain rhand ValveBiped.Bip01_R_Hand knee  0.707 0.707 0.000
$ikchain lhand ValveBiped.Bip01_L_Hand knee  0.707 0.707 0.000
$ikchain rfoot ValveBiped.Bip01_R_Foot knee  0.707 -0.707 0.000
$ikchain lfoot ValveBiped.Bip01_L_Foot knee  0.707 -0.707 0.000
