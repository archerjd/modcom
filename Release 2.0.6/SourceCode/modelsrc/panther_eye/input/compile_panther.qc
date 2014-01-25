$cd "C:\Users\Bryan Haley\Documents\modcom_source\modelsrc\panther_eye\input\"
$modelname "panthereye.mdl"
$model "studio" "panther_reference.smd"

$cdmaterials "\models\panthereye\"
$hboxset "default"
$hbox 0 "Bip01 Pelvis" -15.310  -9.190  -10.530  6.700  8.470  10.260
$hbox 0 "Bip01 L Leg" -2.820  -7.080  -4.470  22.040  9.600  4.980
$hbox 0 "Bip01 L Leg1" -0.710  -4.140  -2.370  13.650  5.610  4.150
$hbox 0 "Bip01 L Leg2" -4.780  -4.990  -3.680  12.590  1.660  3.980
$hbox 0 "Bip01 L Foot" 0.000  -4.470  -6.520  11.720  14.070  6.200
$hbox 0 "Bip01 L Toe0" 0.000  -0.340  -1.580  6.670  5.080  1.060
$hbox 0 "Bip01 L Toe1" 0.000  -0.630  -0.950  9.340  6.600  1.350
$hbox 0 "Bip01 L Toe2" 0.000  -0.230  -0.730  8.670  6.490  1.620
$hbox 0 "Bip01 R Leg" -2.800  -7.050  -4.760  22.060  9.600  4.740
$hbox 0 "Bip01 R Leg1" -0.750  -4.210  -3.870  13.098  5.620  2.870
$hbox 0 "Bip01 R Leg2" -4.770  -4.970  -3.520  12.640  0.000  4.400
$hbox 0 "Bip01 R Foot" 0.000  -4.400  -5.920  11.700  14.510  6.790
$hbox 0 "Bip01 R Toe0" 0.000  -0.320  -1.040  6.700  5.100  0.130
$hbox 0 "Bip01 R Toe1" 0.000  -0.610  -1.270  9.460  6.620  1.030
$hbox 0 "Bip01 R Toe2" 0.000  -0.210  -0.900  8.710  6.500  0.700
$hbox 0 "Bip01 Spine" 0.000  -9.970  -9.570  11.170  12.590  9.600
$hbox 0 "Bip01 Spine1" -1.210  -14.378  -11.800  14.737  13.810  12.230
$hbox 0 "Bip01 Spine2" -1.430  -13.770  -12.750  14.040  17.030  13.180
$hbox 0 "Bip01 Neck" -3.681  -1.870  -10.610  3.475  10.665  4.430
$hbox 0 "Bip01 Head" -1.740  -15.830  -9.790  12.430  17.140  9.920
$hbox 0 "Bip01 L Arm1" 0.000  -4.330  -4.480  15.810  6.770  3.330
$hbox 0 "Bip01 L Arm2" -0.050  -3.810  -4.300  16.260  5.030  4.600
$hbox 0 "Bip01 L Hand" -2.270  -3.900  -6.450  13.310  4.260  5.770
$hbox 0 "Bip01 L Finger0" 0.000  -1.630  -1.610  8.370  3.730  0.680
$hbox 0 "Bip01 L Finger1" 0.000  -1.750  -1.140  9.030  4.110  0.960
$hbox 0 "Bip01 L Finger2" 0.000  -0.260  -1.210  6.810  3.790  0.800
$hbox 0 "Bip01 R Arm1" 0.000  -4.390  -3.010  15.360  6.730  5.020
$hbox 0 "Bip01 R Arm2" 0.000  -3.890  -4.480  16.220  4.990  4.430
$hbox 0 "Bip01 R Hand" -2.080  -4.020  -5.810  13.450  4.260  6.460
$hbox 0 "Bip01 R Finger0" 0.000  -1.750  -0.920  8.560  3.570  1.480
$hbox 0 "Bip01 R Finger1" 0.000  -1.960  -0.930  9.120  3.840  1.190
$hbox 0 "Bip01 R Finger2" 0.000  -1.100  -1.680  6.780  3.450  1.130
$hbox 0 "Dummy04" -1.710  -1.170  -1.790  1.800  0.000  1.780
$hbox 0 "Bone08" -2.610  -6.560  -1.450  0.230  0.000  1.440
$hbox 0 "Bone09" -2.350  -6.482  -1.160  0.000  0.000  1.150
$hbox 0 "Bone10" -1.930  -6.220  -0.710  0.000  0.000  0.690
$hbox 0 "Bone01" 0.000  0.000  -1.720  9.886  12.860  1.780
$hbox 0 "Bone05" -0.110  0.000  -1.020  7.206  7.073  1.020
$hbox 0 "Bip01 Tail" 0.000  -0.007  -1.740  8.250  1.700  1.470
$hbox 0 "Bip01 Tail2" -0.540  -1.950  -1.960  12.370  3.740  1.590
$surfaceprop "alienflesh"
$eyeposition -0.000 0.000 30.000
$illumposition 13.528 -0.224 25.719
$sequence subtle_motion "subtle_motion" ACT_IDLE 1 fps 15.00
$sequence idle_figit "idle_figit" ACT_IDLE 2 fps 15.00
$sequence get_bug "get_bug" ACT_IDLE 3 fps 15.00
$sequence itch "itch" ACT_IDLE 4 fps 15.00
$sequence shakes "shakes" ACT_IDLE 5 fps 15.00
$sequence walk "walk" loop ACT_WALK 1 fps 15.00
$sequence run "run" loop ACT_RUN 1 fps 20.00
$sequence turn_left "turn_left" ACT_TURN_LEFT 1 fps 15.00
$sequence turn_right "turn_right" ACT_TURN_RIGHT 1 fps 15.00
$sequence crawl_on_belly "crawl_on_belly" loop fps 15.00
$sequence crouch_to_crawl "crouch_to_crawl" loop fps 15.00
$sequence crouch_to_jump "crouch_to_jump" ACT_RANGE_ATTACK1 1 fps 15.00 {
  { event 14 11 }
}

$sequence flinch "flinch" ACT_SMALL_FLINCH 1 fps 15.00
$sequence flinch_hard "flinch_hard" ACT_BIG_FLINCH 1 fps 15.00
$sequence flinch_light "flinch_light" ACT_SMALL_FLINCH 2 fps 15.00
$sequence death_drop "death_drop" ACT_DIEBACKWARD 1 fps 15.00
$sequence death_simple "death_simple" ACT_DIESIMPLE 1 fps 15.00
$sequence death_violent "death_violent" ACT_DIEBACKWARD 2 fps 15.00
$sequence attack_primary "attack_primary" ACT_MELEE_ATTACK1 1 fps 20.00 {
  { event 13 8 }
}

$sequence attack_1 "attack_main_claw" ACT_MELEE_ATTACK1 2 fps 20.00 {
  { event 12 7 }
}

$sequence attack_2 "attack_simple_claw" ACT_MELEE_ATTACK1 3 fps 20.00 {
  { event 1 10 }
}

$sequence attack_3 "attack_main_claw" ACT_MELEE_ATTACK1 2 fps 20.00
$sequence attack_4 "attack_simple_claw" ACT_MELEE_ATTACK1 3 fps 20.00
$sequence attack_5 "attack_simple_claw" ACT_MELEE_ATTACK1 3 fps 20.00
$sequence attack_6 "attack_simple_claw" ACT_MELEE_ATTACK1 3 fps 20.00
$sequence charge_start "subtle_motion" ACT_IDLE 1 fps 15.00
$sequence charge_end "subtle_motion" ACT_IDLE 1 fps 15.00
$sequence charge_run "run" loop ACT_RUN 1 fps 20.00
$sequence walk_to_crouch "walk_to_crouch" loop ACT_FLINCH_RIGHTLEG 1 fps 15.00
$sequence walk_to_stand "walk_to_stand" loop fps 15.00
$sequence ragdoll "ragdoll" ACT_DIERAGDOLL 1 fps 30.00
