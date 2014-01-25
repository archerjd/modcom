"resource/ui/LogonPanel.res"
{
	"logon"
	{
		"ControlName"		"CLogonPanel"
		"fieldName"		"logon"
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
		"labelText"		"#logon_title"
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
		"ypos"		"41"
		"wide"		"390"
		"tall"		"125"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#logon_body"
		"textAlignment"		"northwest"
		"wrap"		"1"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"UIClear"
	}
	"existingAccBtn"
	{
		"ControlName"		"TogglePairButton"
		"fieldName"		"existingAccBtn"
		"xpos"		"60"
		"ypos"		"171"
		"wide"		"138"
		"tall"		"31"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#acc_existing"
		"textAlignment"		"center"
		"enabled"		"1"
		"Font"		"UIClear"
		"tabPosition"		"1"
	}
	"newAccBtn"
	{
		"ControlName"		"TogglePairButton"
		"fieldName"		"newAccBtn"
		"xpos"		"226"
		"ypos"		"171"
		"wide"		"138"
		"tall"		"31"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#acc_new"
		"textAlignment"		"center"
		"enabled"		"1"
		"Font"		"UIClear"
		"tabPosition"		"2"
	}
	"username"
	{
		"ControlName"		"TextEntry"
		"fieldName"		"username"
		"xpos"		"153"
		"ypos"		"221"
		"wide"		"145"
		"tall"		"22"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"Font"		"UIClear"
		"tabPosition"		"3"
	}
	"password"
	{
		"ControlName"		"NotifyingTextEntry"
		"fieldName"		"password"
		"xpos"		"153"
		"ypos"		"251"
		"wide"		"145"
		"tall"		"22"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"Font"		"UIClear"
		"tabPosition"		"4"
	}
	"confirmPassword"
	{
		"ControlName"		"TextEntry"
		"fieldName"		"confirmPassword"
		"xpos"		"153"
		"ypos"		"281"
		"wide"		"145"
		"tall"		"22"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"0"
		"enabled"		"1"
		"Font"		"UIClear"
		"tabPosition"		"5"
	}
	"rememberMe"
	{
		"ControlName"		"CheckButton"
		"fieldName"		"rememberMe"
		"xpos"		"280"
		"ypos"		"311"
		"wide"		"18"
		"tall"		"20"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"6"
	}
	"rememberMeLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"rememberMeLabel"
		"xpos"		"140"
		"ypos"		"309"
		"wide"		"140"
		"tall"		"24"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#remember_me"
		"textAlignment"		"east"
		"enabled"		"1"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"UIClear"
	}
	"btnLogin"
	{
		"ControlName"		"Button"
		"fieldName"		"btnLogin"
		"xpos"		"310"
		"ypos"		"221"
		"wide"		"93"
		"tall"		"52"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#login"
		"textAlignment"		"center"
		"enabled"		"1"
		"default"		"1"
		"Font"		"UIClear"
		"command"	"btnlogin"
		"tabPosition"		"7"
	}	
	"nameLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"nameLabel"
		"xpos"		"11"
		"ypos"		"219"
		"wide"		"134"
		"tall"		"24"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#logon_username"
		"textAlignment"		"east"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"UIClear"
	}	
	"passwordLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"passwordLabel"
		"xpos"		"11"
		"ypos"		"250"
		"wide"		"134"
		"tall"		"24"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"labelText"		"#logon_password"
		"textAlignment"		"east"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"UIClear"
	}	
	"passConfLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"passConfLabel"
		"xpos"		"11"
		"ypos"		"281"
		"wide"		"134"
		"tall"		"24"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"0"
		"labelText"		"#logon_passconf"
		"textAlignment"		"east"
		"dulltext"		"0"
		"brighttext"	"1"
		"Font"		"UIClear"
	}
}