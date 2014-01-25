using System;
using System.Collections.Generic;
using System.Text;

namespace ConvarController
{
    class ConVar
    {
        public ConVar(string name, string value)
            : this(name, value, "") { }

        public ConVar(string name, string value, int flags)
            : this(name, value, "") { }

        public ConVar(string name, string value, int flags, string helpText)
            : this(name, value, "") { }

        public ConVar(string name, string value, string helpText)
        {
            this.name = name;
            this.currentValue = this.defaultValue = value;
            this.helpText = helpText;
            List.Add(name, this);

            // if this is a _base, _scale or _power convar, do no more with it
            if (name.EndsWith("_base") )
                Level_Variable.AddPart(this, Level_Variable.PartType.Base);
            else if (name.EndsWith("_scale") )
                Level_Variable.AddPart(this, Level_Variable.PartType.Scale);
            else if (name.EndsWith("_power"))
                Level_Variable.AddPart(this, Level_Variable.PartType.Power);
            else
            {
                CategorizeDiscreteConvar();
            }
        }

        public void CategorizeDiscreteConvar()
        {
            if (name.StartsWith("mod_"))
            {// add me onto my module
                string modname = Module.ParseName(name);
                if (!Module.List.ContainsKey(modname))
                    Module = new Module(modname);
                else
                    Module = Module.List[modname];
                Module.convars.Add(this);

                // if I am this module's _enabled convar, save me as such.
                if (name.Substring(modname.Length+4) == "_enabled")
                    Module.enabled = this;
            }
            else
                MiscList.Add(name, this);
        }

        public Module Module = null;

        private string name, helpText, currentValue, defaultValue;

        public string Name { get { return name; } }
        public string HelpText { get { return helpText; } }
        public string Default { get { return defaultValue; } }
        public string Value { get { return currentValue; } set { currentValue = value; } }
        public bool IsDefault { get { return currentValue == defaultValue; } }

        public static SortedList<string, ConVar> List = new SortedList<string, ConVar>();
        public static SortedList<string, ConVar> MiscList = new SortedList<string, ConVar>();
    }

    class Level_Variable
    {
        internal class Potential
        {
            public Potential(string n)
            {
                name = n;
                cBase = cScale = cPower = null;
            }
            string name;
            public ConVar cBase, cScale, cPower;

            public bool IsComplete { get { return cBase != null && cScale != null && cPower != null; } }

            // called on all remaining potentials at the end of the import ... not got all their parts, so break them up!
            public void BreakUp()
            {
                if ( cBase != null )
                    cBase.CategorizeDiscreteConvar();
                if ( cScale != null )
                    cScale.CategorizeDiscreteConvar();
                if ( cPower != null )
                    cPower.CategorizeDiscreteConvar();
            }
        }

        internal static SortedList<string, Potential> potentialLevelVars = new SortedList<string, Potential>();

        public enum PartType
        {
            Base,
            Scale,
            Power,
        }
        
        public static void AddPart(ConVar c, PartType type)
        {
            string baseName = c.Name.Substring(0, c.Name.LastIndexOf('_'));
            Potential p;
            if ( potentialLevelVars.ContainsKey(baseName) )
                p = potentialLevelVars[baseName];
            else
            {
                p = new Potential(baseName);
                potentialLevelVars.Add(baseName, p);
            }

            switch (type)
            {
                case PartType.Base:
                    p.cBase = c;
                    break;
                case PartType.Scale:
                    p.cScale = c;
                    break;
                case PartType.Power:
                    p.cPower = c;
                    break;
            }

            if (p.IsComplete) // we've got a convar for each bit, so create a proper level variable
            {
                new Level_Variable(baseName, p.cBase, p.cScale, p.cPower, "");
                potentialLevelVars.Remove(baseName);
            }
        }

        private Level_Variable(string name, ConVar cBase, ConVar cScale, ConVar cPower, string description)
        {
            this.name = name;
            this.description = description;

            this.Base = cBase; this.Scale = cScale; this.Power = cPower;

            // add me onto my module, if I'm a module variable
            if (name.StartsWith("mod_"))
            {
                string modname = Module.ParseName(name);
                if (!Module.List.ContainsKey(modname))
                    Module = new Module(modname);
                else
                    Module = Module.List[modname];
                Module.levelVars.Add(this);
            }
            else
                MiscList.Add(name, this);
        }

        public Module Module = null;
        public ConVar Base, Scale, Power;
        private string name, description;
        public string Name { get { return name; } }
        public string Description { get { return description; } }

        public static SortedList<string, Level_Variable> MiscList = new SortedList<string, Level_Variable>();

        public float Value(int level)
        {
            return Combine(Base.Value, Scale.Value, Power.Value, level);
        }

        public float DefaultValue(int level)
        {
            return Combine(Base.Default, Scale.Default, Power.Default, level);
        }

        public static float Combine(string b, string s, string p, int level)
        {
            float fb, fs, fp;
            if (!float.TryParse(b, out fb) || !float.TryParse(s, out fs) || !float.TryParse(p, out fp))
                return 0;
            return fb + fs * (float)Math.Pow(level, fp);
        }

        public bool IsDefault
        {
            get
            {
                return Base.IsDefault && Scale.IsDefault && Power.IsDefault;
            }
        }
    }

    // for collating all level variables & convars related to a given module, and displaying them together.
    class Module
    {
        string name;
        public Module(string name)
        {
            this.name = name;
            List.Add(name, this);
        }

        public string Name { get { return name; } }

        public List<Level_Variable> levelVars = new List<Level_Variable>();
        public List<ConVar> convars = new List<ConVar>();
        public ConVar enabled = null;

        public static SortedList<string, Module> List = new SortedList<string, Module>();

        public bool IsEnabled
        {
            get
            {// check my _enabled module, return its value
                if (enabled != null)
                    return enabled.Value == "1";
                return true;
            }
        }

        public bool IsDefault
        {
            get
            {
                foreach (ConVar c in convars)
                    if (!c.IsDefault)
                        return false;
                foreach (Level_Variable l in levelVars)
                    if (!l.IsDefault)
                        return false;
                return true;
            }
        }

        internal static string ParseName(string name)
        {
            int modNameStart = 4;
            int modNameEnd = name.IndexOf('_', modNameStart);
            if (modNameEnd == -1)
                modNameEnd = name.Length;
            return name.Substring(modNameStart, modNameEnd - modNameStart);
        }
    }
}