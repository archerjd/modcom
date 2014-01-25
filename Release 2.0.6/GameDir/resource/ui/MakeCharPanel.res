"resource/ui/MakeCharPanel.res"
{
	"makechar"
	{
		"ControlName"		"CMakeCharPanel"
		"fieldName"		"makechar"
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
		"labelText"		"#makechar_title"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"BigTitle"
	}
	"bodyLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"bodyLabel"
		"xpos"		"22"
		"ypos"		"37"
		"wide"		"390"
		"tall"		"48"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#makechar_body"
		"textAlignment"		"northwest"
		"wrap"		"1"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"UIClear"
	}

	"charname"
	{
		"ControlName"		"TextEntry"
		"fieldName"		"charname"
		"xpos"		"80"
		"ypos"		"79"
		"wide"		"139"
		"tall"		"22"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"Font"		"UIClear"
		"tabPosition"		"1"
	}
	"nameLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"nameLabel"
		"xpos"		"46"
		"ypos"		"78"
		"wide"		"30"
		"tall"		"24"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#make_char_name"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"UIClear"
	}
	"charclass"
	{
		"ControlName"		"TextEntry"
		"fieldName"		"charclass"
		"xpos"		"80"
		"ypos"		"123"
		"wide"		"139"
		"tall"		"22"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"0"
		"enabled"		"0"
		"Font"		"UIClear"
		"tabPosition"		"2"
	}
	"classLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"classLabel"
		"xpos"		"1"
		"ypos"		"122"
		"wide"		"70"
		"tall"		"24"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"0"
		"labelText"		"#make_char_class"
		"textAlignment"		"east"
		"dulltext"		"1"
		"brighttext"	"0"
		"Font"		"UIClear"
	}
	
	"appearanceLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"appearanceLabel"
		"xpos"		"242"
		"ypos"		"78"
		"wide"		"160"
		"tall"		"24"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#make_char_appearance"
		"textAlignment"		"center"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"UIClear"
	}
	"imageBackground"
	{
		"ControlName"		"Button"
		"fieldName"		"imageBackground"
		"xpos"		"272"
		"ypos"		"106"
		"wide"		"100"
		"tall"		"100"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		""
		"textAlignment"	"center"
		"enabled"		"0"
	}
	"charimage" // the 256x256 images only use the top left 200x200 for image data
	{
		"ControlName"	"BitmapImagePanel"
		"fieldName"		"charimage"
		"xpos"			"272"
		"ypos"			"106"
		"wide"			"128"
		"tall"			"128"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"1"
	}
	"btnPrev"
	{
		"ControlName"		"Button"
		"fieldName"		"btnPrev"
		"xpos"		"272"
		"ypos"		"206"
		"wide"		"50"
		"tall"		"30"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"w"
		"textAlignment"	"center"
		"enabled"		"1"
		"Font"		"Marlett"
		"command"	"prev"
		"tabPosition"	"3"
	}
	"btnNext"
	{
		"ControlName"		"Button"
		"fieldName"		"btnNext"
		"xpos"		"322"
		"ypos"		"206"
		"wide"		"50"
		"tall"		"30"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"4"
		"textAlignment"	"center"
		"enabled"		"1"
		"Font"		"Marlett"
		"command"	"next"
		"tabPosition"	"4"
	}
	
	"btnCancel"
	{
		"ControlName"		"Button"
		"fieldName"		"btnCancel"
		"xpos"		"234"
		"ypos"		"269"
		"wide"		"84"
		"tall"		"63"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#cancel"
		"textAlignment"	"center"
		"enabled"		"1"
		"Font"		"UIClear"
		"command"	"cancel"
		"tabPosition"	"5"
	}
	"btnCreate"
	{
		"ControlName"		"Button"
		"fieldName"		"btnCreate"
		"xpos"		"325"
		"ypos"		"269"
		"wide"		"84"
		"tall"		"63"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#make_char"
		"textAlignment"	"center"
		"enabled"		"1"
		"Font"		"UIClear"
		"command"	"create"
		"tabPosition"	"6"
		"default"	"1"
	}
	
	"labelFaction"
	{
		"ControlName"		"Label"
		"fieldName"		"labelFaction"
		"xpos"		"40"
		"ypos"		"269"
		"wide"		"46"
		"tall"		"13"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#faction"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"UIClear"
	}	
	"ddlFaction"
	{
		"ControlName"		"ComboBox"
		"fieldName"		"ddlFaction"
		"xpos"		"93"
		"ypos"		"269"
		"wide"		"122"
		"tall"		"18"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"textAlignment"	"west"
		"enabled"		"1"
		"Font"		"UIClear"
		"command"	"faction"
		"tabPosition"	"0"
	}
	
	"factionDesc"
	{
		"ControlName"		"Label"
		"fieldName"		"factionDesc"
		"xpos"		"32"
		"ypos"		"281"
		"wide"		"198"
		"tall"		"60"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#faction_resistance_desc"
		"textAlignment"		"northwest"
		"wrap"		"1"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"UIClear"
	}
}