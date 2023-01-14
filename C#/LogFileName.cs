using System;
using System.Text;
using System.IO;
using System.Threading;
using System.Diagnostics;

namespace net.jurasz.tools
{
	// /////////////////////////////////////////////////////////////////////////////////
	//								C l a s s											
	/// <summary>
	/// Klasa opisujaca lokalizacje pliku logu
	/// </summary>
	public class CLogFileName
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
        #region Definitions
        #endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
        #region Members

        private string m_sName;
		private string m_sPrefix	= "";
		private string m_sSuffix	= "";
		private string m_sExtension	= "log";
		private string m_sDirectory	= "";
		
		private string m_sLogFullPathName;

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
		#region Constructors

		/// <summary>
		/// Konstruktor. Tworzy nazwe postaci [Directory]\[Prefix]_[Name]_[Suffix].[Extension]
		/// </summary>
		/// <param name="i_sDirectory">Katalog</param>
		/// <param name="i_sPrefix">Przedrostek nazwy pliku</param>
		/// <param name="i_sName">Glowna czesc nazwy pliku</param>
		/// <param name="i_sSuffix">Przyrostek nazwy pliku</param>
		/// <param name="i_sExtension">Rozszerzenie nazwy pliku</param>
		public CLogFileName(
			string i_sDirectory,
			string i_sPrefix,
			string i_sName,
			string i_sSuffix,
			string i_sExtension
			)
		{
			Debug.Assert(i_sName != null); // UWAGA! W tej klasie nie WOLNO korzystac z CPfxAssert
			m_sName = i_sName;
			if (i_sPrefix != null)
				m_sPrefix = i_sPrefix;

			if (i_sSuffix != null)
				m_sSuffix = i_sSuffix;
			
			if (i_sExtension != null)
				m_sExtension = i_sExtension;

			if (i_sDirectory != null)
				m_sDirectory = i_sDirectory;

			SetLogFullName();
		}

		/// <summary>
		/// Konstruktor. Towrzy nazwe postaci [Name].log
		/// </summary>
		/// <param name="i_sName"></param>
		public CLogFileName(
			string i_sName
		)
		{
			Debug.Assert(i_sName != null);
			m_sName = i_sName;
			SetLogFullName();
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface

		/// <summary>
		/// Zwraca pelna nazwe pliku (ze scie¿ka o ile istnieje)
		/// </summary>
		public string p_sLogName
		{
			get
			{
				return m_sLogFullPathName;
			}
		}

		/// <summary>
		/// Zmiana katalogu logu
		/// </summary>
		/// <param name="i_sDirectory">Nowa scie¿ka</param>
		public void ChangeLogDir(
			string i_sDirectory
			)
		{
			if (i_sDirectory != null)
			{
				m_sDirectory = i_sDirectory;
				SetLogFullName();
			}
		}

		/// <summary>
		/// Zmiana glonej czesci nazwy logu
		/// </summary>
		/// <param name="i_sName">Nowa nazwa logu</param>
		public void ChangeLogName(
			string i_sName
			)
		{
			if (i_sName != null)
			{
				m_sName = i_sName;
				SetLogFullName();
			}
		}

		/// <summary>
		/// Zmiana nazwy logu
		/// </summary>
		/// <param name="i_sPrefix">Przedrostek</param>
		/// <param name="i_sName">Glowna czesc nazwy</param>
		/// <param name="i_sSuffix">Przyrostek</param>
		/// <param name="i_sExtension">Rozszerzenie</param>
		public void ChangeLogName(
			string i_sPrefix,
			string i_sName,
			string i_sSuffix,
			string i_sExtension
			)
		{
			if (i_sPrefix != null)
				m_sPrefix = i_sPrefix;

			if (i_sName != null)
				m_sName = i_sName;

			if (i_sSuffix != null)
				m_sSuffix = i_sSuffix;

			if (i_sExtension != null)
				m_sExtension = i_sExtension;

			SetLogFullName();
		}

		/// <summary>
		/// Ustawia nazwe logu jesli nie bylo
		/// </summary>
		/// <param name="i_sLogName"></param>
		public void AssureLogName(
			string i_sLogName
			)
		{
			CPfxAssert.StringNotEmpty(i_sLogName);
			if (m_sName == null || m_sName.Length == 0)
				m_sName = i_sLogName;
			SetLogFullName();
		}
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
		#region PrivateInterface

		/// <summary>
		/// Ustawia pelna nazwe pliku logu w zmienej pomocniczej
		/// </summary>
		private void SetLogFullName()
		{
			StringBuilder oName = new StringBuilder(100);
			
			if (m_sDirectory.Length > 0)
			{
				oName.Append(m_sDirectory);
				if (!m_sDirectory.EndsWith("\\"))
					oName.Append("\\");
			}

			if (m_sPrefix.Length > 0)
			{
				oName.Append(m_sPrefix);
				oName.Append("_");
			}
			oName.Append(m_sName);
			if (m_sSuffix.Length > 0)
			{
				oName.Append("_");
				oName.Append(m_sSuffix);
			}
			if (m_sExtension.Length > 0)
			{
				oName.Append(".");
				oName.Append(m_sExtension);
			}

			m_sLogFullPathName = oName.ToString();
		}
		#endregion
	}
}
