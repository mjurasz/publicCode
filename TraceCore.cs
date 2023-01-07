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
	/// Klasa bazowa logow
	/// </summary>
	/// <remarks>
	/// Tymczasowo ograniczony zestaw metod z komunikatami o roznej szerokosci
	/// </remarks>
	public class CTraceCore : CTraceWriter
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
		#region Definitions

		/// <summary>
		/// Typ kolumny logu
		/// </summary>
		public enum EMessageFieldType
		{
			eMessage		= 0,	// komunikat
			eDate			= 1,	// data
			eTime			= 2,	// czas
			eThreadId		= 3,	// identyfikator watku
			eUserDefined	= 4		// kolumna zdefiniowana przez uzytkownika
		}

		/// <summary>
		/// Klasa opisujaca kolumne logu
		/// </summary>
		public class CMessageField
		{
			public EMessageFieldType	m_eType;		// typ kolumny
			public string				m_sFormatString;// lancuch formatujacy	
			public string				m_sSeparator;	// separator do nastepnej kolumny

			public CMessageField()
			{
				m_eType				=  EMessageFieldType.eMessage;
				m_sFormatString		=  "{0}";
				m_sSeparator		=  "\t";

			}

			public CMessageField(
				EMessageFieldType i_eType
			)
			{
				m_eType				=  i_eType;
				m_sFormatString		=  "{0}";
				m_sSeparator		=  "\t";
			}

			public CMessageField(
				EMessageFieldType	i_eType,
				string				i_sFormatString
			)
			{
				m_eType				=  i_eType;
				m_sFormatString		=  i_sFormatString;
				m_sSeparator		=  "\t";
			}

			public CMessageField(
				EMessageFieldType	i_eType,
				string				i_sFormatString,
				string				i_sSeparator
				)
			{
				m_eType				=  i_eType;
				m_sFormatString		=  i_sFormatString;
				m_sSeparator		=  i_sSeparator;
			}

			public void Set(
				EMessageFieldType	i_eType,
				string				i_sFormatString,
				string				i_sSeparator
				)
			{
				m_eType				=  i_eType;
				m_sFormatString		=  i_sFormatString;
				m_sSeparator		=  i_sSeparator;
			}

			public void Set(
				EMessageFieldType	i_eType,
				string				i_sFormatString
				)
			{
				m_eType				=  i_eType;
				m_sFormatString		=  i_sFormatString;
			}

			public void Set(
				EMessageFieldType i_eType
			)
			{
				m_eType = i_eType;
			}

		}

	#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
		#region Members

		/// <summary>
		/// globalny wylacznik logowania (dla wszystkich logow)
		/// </summary>
		private static bool		sm_bGlobalLogOn = true;
		/// <summary>
		/// lokalny wylacznik logowania
		/// </summary>
		private bool			m_bLogOn		= true;
		/// <summary>
		/// szerokosc komunikatu (0 - bez ograniczen)
		/// </summary>
		private int				m_nMessageWidth = 0;
		/// <summary>
		/// definicje kolumn
		/// </summary>
		private CMessageField[]	m_oMessageFieldsArray;
		
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
		#region Constructors

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sDirectory"></param>
		/// <param name="i_sLogNamePrefix"></param>
		/// <param name="i_sLogName"></param>
		/// <param name="i_sLogNameSuffix"></param>
		/// <param name="i_sLogNameExtension"></param>
		/// <param name="i_nMessageWidth"></param>
		/// <param name="i_oMessageSegmentsArray"></param>
		/// <param name="i_bAppendLog"></param>
		/// <param name="i_nBufferSize"></param>
		/// <param name="i_bMultiLineLog"></param>
		protected CTraceCore(
			string			i_sDirectory,
			string			i_sLogNamePrefix,
			string			i_sLogName,
			string			i_sLogNameSuffix,
			string			i_sLogNameExtension,
			int				i_nMessageWidth,
			CMessageField[]	i_oMessageSegmentsArray,
			bool			i_bAppendLog,
			int				i_nBufferSize,
			bool			i_bMultiLineLog
		) : base(
			i_nBufferSize,
			i_bAppendLog, 
			i_bMultiLineLog,
			i_sDirectory,
			i_sLogNamePrefix,
			i_sLogName,
			i_sLogNameSuffix,
			i_sLogNameExtension
		)
		{
			try
			{
				m_nMessageWidth = i_nMessageWidth;
				
				m_oMessageFieldsArray = i_oMessageSegmentsArray;

				if (m_oMessageFieldsArray == null)
				{
					m_oMessageFieldsArray = new CMessageField[1];
					m_oMessageFieldsArray[0] = new CMessageField(EMessageFieldType.eMessage);
				}
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sDirectory"></param>
		/// <param name="i_sLogNamePrefix"></param>
		/// <param name="i_sLogName"></param>
		/// <param name="i_sLogNameSuffix"></param>
		/// <param name="i_sLogNameExtension"></param>
		/// <param name="i_nMessageWidth"></param>
		/// <param name="i_oMessageSegmentsArray"></param>
		/// <param name="i_bAppendLog"></param>
		/// <param name="i_nBufferSize"></param>
		protected CTraceCore(
			string			i_sDirectory,
			string			i_sLogNamePrefix,
			string			i_sLogName,
			string			i_sLogNameSuffix,
			string			i_sLogNameExtension,
			int				i_nMessageWidth,
			CMessageField[]	i_oMessageSegmentsArray,
			bool			i_bAppendLog,
			int				i_nBufferSize
		):
			this(
				i_sDirectory,
				i_sLogNamePrefix,
				i_sLogName,
				i_sLogNameSuffix,
				i_sLogNameExtension,
				i_nMessageWidth,
				i_oMessageSegmentsArray,
				i_bAppendLog,
				i_nBufferSize,
				true
			)
		{
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface

		/// <summary>
		/// Pobiera / ustawia wartosc globalnego wylacznika logow
		/// </summary>
		public static bool p_bGlobalLogOn
		{
			get 
			{
				return sm_bGlobalLogOn;
			}
			set
			{
				sm_bGlobalLogOn = value;
			}
		}

		/// <summary>
		/// Pobiera / ustawia wartosc lokalnego wylacznika logu (tej instancji)
		/// </summary>
		public bool p_bLogOn
		{
			get 
			{
				return m_bLogOn;
			}
			set
			{
				m_bLogOn = value;
			}
		}
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
		#region PrivateInterface

		/// <summary>
		/// Zapis wiersza logu. Tylko komunikat, bez parametrow i kolumn uzytkownika
		/// </summary>
		/// <param name="i_sMessage">Komunikat</param>
		protected void Trace(
			string i_sMessage
			)
		{
			try
			{
				if (!m_bLogOn || !sm_bGlobalLogOn)
					return;

				string sLine = PrepareMessage(null, i_sMessage);
				Write(sLine);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// Zapis wiersza logu. Komunikat z parametrami  - bez kolumn uzytkownika
		/// </summary>
		/// <param name="i_sMessage">Komunikat</param>
		/// <param name="i_oParams">Lista parametrow komunikatu</param>
		protected void Trace(
			string			i_sMessage,
			params object[] i_oParams
			)
		{
			try
			{
				if (!m_bLogOn || !sm_bGlobalLogOn)
					return;

				string sMessage = FormatMessage(i_sMessage, i_oParams);

				string sLine	= PrepareMessage(null, sMessage);
				Write(sLine);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// Zapis wiersza logu. Komunikat bez parametrow oraz kolumny uzytkownika
		/// </summary>
		/// <param name="i_oUserFieldsArray">Tablica wartosci dla kolumn uzytkownika</param>
		/// <param name="i_sMessage">Komunikat</param>
		protected void Trace(
			object[]		i_oUserFieldsArray,
			string			i_sMessage
			)
		{
			try
			{
				if (!m_bLogOn || !sm_bGlobalLogOn)
					return;

				string sLine = PrepareMessage(i_oUserFieldsArray, i_sMessage);
				Write(sLine);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// Zapis wiersza logu. Komunikat z parametrami oraz kolumnami uzytkownika
		/// </summary>
		/// <param name="i_oUserFieldsArray">Tablica wartosci dla kolumn uzytkownika</param>
		/// <param name="i_sMessage">Komunikat</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		protected void Trace(
			object[]		i_oUserFieldsArray,
			string			i_sMessage,
			params object[] i_oParams
			)
		{
			try
			{
				if (!m_bLogOn || !sm_bGlobalLogOn)
					return;

				string sMessage = FormatMessage(i_sMessage, i_oParams);

				string sLine = PrepareMessage(i_oUserFieldsArray, sMessage);
				Write(sLine);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// Zapis wiersza logu. Komunikat o roznej szerokosci z parametrami i kolumnami uzytkownika
		/// </summary>
		/// <param name="i_oUserFieldsArray">Tablica wartosci dla kolumn uzytkownika</param>
		/// <param name="i_sMessageElements">Komunikat w wersji o roznej szerokosci</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		protected void Trace(
			object[]		i_oUserFieldsArray,
			string[]		i_sMessageElements,
			params object[] i_oParams
			)
		{
			try
			{
				if (!m_bLogOn || !sm_bGlobalLogOn)
					return;

				int nMessageWidth = i_sMessageElements.Length;
				if (m_nMessageWidth != 0 && m_nMessageWidth < nMessageWidth)
					nMessageWidth = m_nMessageWidth;

				StringBuilder oMessageFormat = new StringBuilder(200);
				for (int i=0; i<nMessageWidth; i++)
					oMessageFormat.Append(i_sMessageElements[i]);

				string sMessageFormat = oMessageFormat.ToString();
				string sMessage = FormatMessage(sMessageFormat, i_oParams);
				string sLine = PrepareMessage(i_oUserFieldsArray, sMessage);
				Write(sLine);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// Formatowanie pola komunikatu (jednej kolumny)
		/// </summary>
		/// <param name="i_oUserFieldsArray">Tablica wartosci dla kolumn uzytkownika</param>
		/// <param name="i_sMessage">Komunikat</param>
		/// <param name="i_oNow">Data i czas wpisu</param>
		/// <param name="i_eFieldType">Typ pola</param>
		/// <param name="io_nUserField">indeks kolumny uzytkownika</param>
		/// <returns>sformatowane pole komunikatu</returns>
		private string PrepareMessageField(
			object[]			i_oUserFieldsArray,
			string				i_sMessage,
			DateTime			i_oNow,
			EMessageFieldType	i_eFieldType,
			ref int				io_nUserField
			)
		{
			string sField;
			switch (i_eFieldType)
			{
				case EMessageFieldType.eDate:
					sField = string.Format("{0:D2}/{1:D2}/{2}", i_oNow.Day, i_oNow.Month, i_oNow.Year);
					break;

				case EMessageFieldType.eMessage:
					sField = i_sMessage;
					break;

				case EMessageFieldType.eThreadId:
					// sField = string.Format("{0:X}",AppDomain.GetCurrentThreadId());
                    sField = string.Format("{0:X}", System.Threading.Thread.CurrentThread.ManagedThreadId);
					break;

				case EMessageFieldType.eTime:
					sField = string.Format("{0:D2}:{1:D2}:{2:D2}.{3:D3}", i_oNow.Hour, i_oNow.Minute, i_oNow.Second, i_oNow.Millisecond);
					break;
					
				case EMessageFieldType.eUserDefined:
					if (i_oUserFieldsArray == null)
						sField = "FIELD MISSING";
					else
						if (io_nUserField < i_oUserFieldsArray.Length)
					{
						sField = i_oUserFieldsArray[io_nUserField].ToString();
						io_nUserField++;
					}
					else
						sField = "FIELD MISSING";
					break;

				default:
					sField = "UNKNOWN FIELD TYPE";
					Debug.Assert(false, "Unknown enum");
					break;
			}
			return sField;
		}
		
		/// <summary>
		/// Przygotowanie wiersza logu
		/// </summary>
		/// <param name="i_oUserFieldsArray">Tablica wartosci dla kolumn uzytkownika</param>
		/// <param name="i_sMessage">Sformatowany komunikat</param>
		/// <returns>Wiersz logu</returns>
		private string PrepareMessage(
			object[] i_oUserFieldsArray,
			string	 i_sMessage
			)
		{
			DateTime oNowCET		= GetCurrentCETDateTime(); // DateTime.Now;
			int nUserDefinedField	= 0;
			StringBuilder oLine		= new StringBuilder(300);
			
			for (int i=0; i<m_oMessageFieldsArray.Length; i++)
			{
				string sField = PrepareMessageField(
					i_oUserFieldsArray,
					i_sMessage,
					oNowCET,
					m_oMessageFieldsArray[i].m_eType,
					ref nUserDefinedField
					);
				
                oLine.Append(string.Format(m_oMessageFieldsArray[i].m_sFormatString, sField));
				if (i != m_oMessageFieldsArray.Length - 1)
					oLine.Append(m_oMessageFieldsArray[i].m_sSeparator);
			}
			return oLine.ToString();
		}

		private DateTime GetCurrentCETDateTime()
		{
			// ReadOnlyCollection<TimeZoneInfo> zones = TimeZoneInfo.GetSystemTimeZones();
			// Console.WriteLine("The local system has the following {0} time zones", zones.Count);

			// foreach (TimeZoneInfo zone in zones)
			//    Console.WriteLine(zone.Id);  

			DateTime dtCurrentCET = TimeZoneInfo.ConvertTime(
						DateTime.Now,
						TimeZoneInfo.FindSystemTimeZoneById("Central European Standard Time")
					);

			return dtCurrentCET;
		}

		/// <summary>
		/// Zapis wiersza logu. Komunikat z parametrami  - bez kolumn uzytkownika
		/// </summary>
		/// <param name="i_sMessage">Komunikat</param>
		/// <param name="i_oParams">Lista parametrow komunikatu</param>
		private string FormatMessage(
			string			i_sMessage,
			params object[] i_oParams
			)
		{
			try
			{
				return string.Format(i_sMessage, i_oParams);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
				// TODO - DEBUG
				//Debug.Assert(false, "Wyjatek przekroczenia zakresu podczas formatowania lancucha");
				throw;
			}
		}

		#endregion
	}
}
