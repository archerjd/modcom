"resource/ui/PickCharPanel.res"
{
	"pickchar"
	{
		"ControlName"		"CPickCharPanel"
		"fieldName"		"pickchar"
		"xpos"		"107"
		"ypos"		"67"
		"wide"		"425"
		"tall"		"346"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"0"
		"enabled"		"1"
		"tabPosition"		"0"
		"setTitleBarVisible"	"0"
		"title"		""
	}
	"titleLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"titleLabel"
		"xpos"		"33"
		"ypos"		"14"
		"wide"		"390"
		"tall"		"29"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#pickchar_title"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"BigTitle"
	}
	"bodyLabel1"
	{
		"ControlName"		"Label"
		"fieldName"		"bodyLabel1"
		"xpos"		"22"
		"ypos"		"46"
		"wide"		"390"
		"tall"		"12"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#pickchar_body1"
		"textAlignment"		"northwest"
		"wrap"		"1"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"UIClear"
	}
	
	"bodyLabel2"
	{
		"ControlName"		"Label"
		"fieldName"		"bodyLabel2"
		"xpos"		"22"
		"ypos"		"60"
		"wide"		"390"
		"tall"		"12"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#pickchar_body2"
		"textAlignment"		"northwest"
		"wrap"		"1"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"UIClear"
	}

	"btnChar1"
	{
		"ControlName"		"Button"
		"fieldName"		"btnChar1"
		"xpos"		"24"
		"ypos"		"88"
		"wide"		"117"
		"tall"		"168"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		""
		"textAlignment"	"north"
		"enabled"		"1"
		"Font"		"UIClear"
		"command"	"char1"
		"tabPosition"	"1"
	}
	"btnChar2"
	{
		"ControlName"		"Button"
		"fieldName"		"btnChar2"
		"xpos"		"154"
		"ypos"		"88"
		"wide"		"117"
		"tall"		"168"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		""
		"textAlignment"	"north"
		"enabled"		"1"
		"Font"		"UIClear"
		"command"	"char2"
		"tabPosition"	"2"
	}
	"btnChar3"
	{
		"ControlName"		"Button"
		"fieldName"		"btnChar3"
		"xpos"		"284"
		"ypos"		"88"
		"wide"		"117"
		"tall"		"168"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		""
		"textAlignment"	"north"
		"enabled"		"1"
		"Font"		"UIClear"
		"command"	"char3"
		"tabPosition"	"3"
	}
	"imgChar1" // image is 256x256, but only uses top left 200x200
	{
		"ControlName"		"BitmapImagePanel"
		"fieldName"		"imgChar1"
		"xpos"		"25"
		"ypos"		"141"
		"wide"		"147"
		"tall"		"147"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"0"
	}
	"imgChar2" // image is 256x256, but only uses top left 200x200
	{
		"ControlName"		"BitmapImagePanel"
		"fieldName"		"imgChar2"
		"xpos"		"155"
		"ypos"		"141"
		"wide"		"147"
		"tall"		"147"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"0"
	}
	"imgChar3" // image is 256x256, but only uses top left 200x200
	{
		"ControlName"		"BitmapImagePanel"
		"fieldName"		"imgChar3"
		"xpos"		"285"
		"ypos"		"141"
		"wide"		"147"
		"tall"		"147"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"0"
	}
	
	"btnPrev"
	{
		"ControlName"		"Button"
		"fieldName"		"btnPrev"
		"xpos"		"0"
		"ypos"		"148"
		"wide"		"23"
		"tall"		"48"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"w"
		"textAlignment"	"center"
		"wrap"		"1"
		"enabled"		"1"
		"Font"		"Marlett"
		"command"	"prev"
		"tabPosition"	"4"
	}
	"btnNext"
	{
		"ControlName"		"Button"
		"fieldName"		"btnNext"
		"xpos"		"402"
		"ypos"		"148"
		"wide"		"23"
		"tall"		"48"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"4"
		"textAlignment"	"center"
		"wrap"		"1"
		"enabled"		"1"
		"Font"		"Marlett"
		"command"	"next"
		"tabPosition"	"5"
	}
	
	"btnNewChar"
	{
		"ControlName"		"Button"
		"fieldName"		"btnNewChar"
		"xpos"		"24"
		"ypos"		"269"
		"wide"		"84"
		"tall"		"63"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#new_char"
		"textAlignment"	"center"
		"enabled"		"1"
		"Font"		"UIClear"
		"command"	"newchar"
		"tabPosition"	"6"
	}
	"btnDelete"
	{
		"ControlName"		"ToggleButton"
		"fieldName"		"btnDelete"
		"xpos"		"122"
		"ypos"		"269"
		"wide"		"84"
		"tall"		"63"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#delete"
		"textAlignment"	"center"
		"enabled"		"1"
		"Font"		"UIClear"
		"tabPosition"	"7"
	}
	"btnSpectate"
	{
		"ControlName"		"Button"
		"fieldName"		"btnSpectate"
		"xpos"		"220"
		"ypos"		"269"
		"wide"		"84"
		"tall"		"63"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#spectate"
		"textAlignment"	"center"
		"enabled"		"1"
		"Font"		"UIClear"
		"command"	"spec"
		"tabPosition"	"8"
	}
	"btnLogoff"
	{
		"ControlName"		"Button"
		"fieldName"		"btnLogoff"
		"xpos"		"318"
		"ypos"		"269"
		"wide"		"84"
		"tall"		"63"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#logoff"
		"textAlignment"	"center"
		"enabled"		"1"
		"Font"		"UIClear"
		"command"	"logoff"
		"tabPosition"	"9"
	}
}