using System;
using System.Collections.Generic;
using System.Data.Sql;
using System.Data;
using System.Text;
using System.Resources;
using System.Reflection;
using System.Globalization;
using System.Web;
using System.Web.UI.WebControls;
using System.Web.Configuration;
using System.Configuration;
using Microsoft.Win32;

namespace net.jurasz.tools
{
    public static class CResourceOperations
    {
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
        #region Definitions

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
        #region Members

        #endregion 

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
        #region Constructors

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
        #region PublicInterface
        
        /// <summary>
        /// 
        /// </summary>
        /// <param name="i_sKey">key</param>
        /// <returns>name from resource</returns>
        public static string GetResourceString(
            string i_sKey
            )
        {
            return GetResourceString(
                i_sKey,
                Assembly.GetCallingAssembly()
            );
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="i_sKey">key</param>
        /// <param name="i_oAssembly">assembly</param>
        /// <returns>name from resources</returns>
        public static string GetResourceString(
            string i_sKey,
            Assembly i_oAssembly
            )
        {
            return GetResourceString(
                i_sKey,
                i_oAssembly,
                CGlobalDefs.def_sDefaultResourceNamespace
                );
        }

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
        #region PrivateInterface

        /// <summary>
        /// 
        /// </summary>
        /// <param name="i_sKey"></param>
        /// <param name="i_oAssembly"></param>
        /// <param name="i_sResourceNamespace"></param>
        /// <returns></returns>
        private static string GetResourceString(
            string i_sKey,
            Assembly i_oAssembly,
            string i_sResourceNamespace
            )
        {
            CPfxAssert.IsNotNull(i_sResourceNamespace, "i_sResourceNamespace == null");
            CPfxAssert.IsNotNull(i_oAssembly, "i_oAssembly == null");

            ResourceManager oResMgr;

            if (i_sKey == null)
                return "";

            if (i_sResourceNamespace == null || i_oAssembly == null)
                return i_sKey;

            string sKey = null;

            try
            {
                oResMgr = new System.Resources.ResourceManager(
                    i_sResourceNamespace,
                    i_oAssembly
                    );
                sKey = oResMgr.GetString(i_sKey);
            }
            catch (Exception e)
            {
                CExceptionManager.LogException(e);
            }

            if (sKey == null)
                return i_sKey;
            else
                return sKey;
        }

        #endregion
    }

    public static class CGlobalDefs
    {
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
        #region Definitions
        
        public const string def_sDefaultResourceNamespace = "com.pregis.ResourceNamespace";
        public const string def_sVirtualPath = @"/Site";

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
        #region Members

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
        #region Constructors

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
        #region PublicInterface

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
        #region PrivateInterface

        #endregion
    }

    public static class CAsserts
    {
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
        #region Definitions

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
        #region Members

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
        #region Constructors

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
        #region PublicInterface

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
        #region PrivateInterface

        #endregion
    }

    public static class CAppSettingsOperations
    {
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
        #region Definitions

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
        #region Members

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
        #region Constructors

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
        #region PublicInterface

        public static string GetAppSettingsValue(string i_strKeyName)
        {
            string strKeyValue = string.Empty;

            try
            {
                // CPfxAssert.IsNotNull(ConfigurationManager.AppSettings[i_strKeyName].ToString());
                // -- WERSJA DLA .NET 2.0 i 3.5
                // strKeyValue = ConfigurationManager.AppSettings[i_strKeyName].ToString();

                // -- WERSJA DLA .NET 4.0
                //Configuration config = ConfigurationManager.OpenExeConfiguration(ConfigurationUserLevel.None);
                //ConfigurationSectionGroup applicationSectionGroup = config.GetSectionGroup("applicationSettings");
                
                //ConfigurationSection applicationConfigSection = applicationSectionGroup.Sections["PX3_XMLParserTool.Properties.Settings"];
                
                //ClientSettingsSection clientSection = (ClientSettingsSection)applicationConfigSection;
                //SettingElement applicationSetting = clientSection.Settings.Get(i_strKeyName);

                //CPfxAssert.IsNotNull(applicationSetting);
                //strKeyValue = applicationSetting.Value.ValueXml.InnerXml;

                // WERSJA z rejestrem systemowym

                using (RegistryKey Key = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\eProphet\WHS\"))
                {
                    if (Key != null)
                        strKeyValue = Key.GetValue(i_strKeyName).ToString();
                }
            }
            catch (Exception e)
            {
                CExceptionManager.LogException(e);
            }

            return strKeyValue;
        }        

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
        #region PrivateInterface

        #endregion
    }

    public static class CGUIOperations
    {
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
        #region Definitions

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
        #region Members

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
        #region Constructors

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
        #region PublicInterface

        public static void InitializeDropDownList(DropDownList i_ddList, DataTable i_dtSource, bool i_bWithNA)
        {
            try
            {
                for (int i = 0; i < i_dtSource.Rows.Count; i++)
                {
                    if (i == 0 && i_bWithNA)
                        i_ddList.Items.Add("N/A");

                    i_ddList.Items.Add(i_dtSource.Rows[i]["CompanyKiniName"].ToString());
                }
            }
            catch (Exception e)
            {
                CExceptionManager.LogException(e);
            }
        }

        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
        #region PrivateInterface

        #endregion
    }
}
