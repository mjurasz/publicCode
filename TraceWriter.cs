using System;
using System.IO;
using System.Text;

namespace net.jurasz.tools
{
	// /////////////////////////////////////////////////////////////////////////////////
	//								C l a s s											
	/// <summary>
	/// Klasa buforu logowania
	/// </summary>
    public class CTraceWriter : CDisposableBase
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
		#region Definitions
		/// <summary>
		/// 
		/// </summary>
		public const int def_nWriteLinesBetweenSizeCheck	= 10;
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
		#region Members
		private static CLogContainer	sm_oLogContainer = new CLogContainer();
        /// <summary>
        /// bufor - tablica stringow
        /// </summary>
		private	string[]		m_sBuffer;
		/// <summary>
		/// wskaünik bufora
		/// </summary>
		private int				m_nPos = -1;
		/// <summary>
		/// dane pliku logowania
		/// </summary>
		public CLogFileName	m_oLogName;
		/// <summary>
		/// obiekt pomocniczy do zmiany Unicode w zwykly tekst
		/// </summary>
		private Encoding		m_oEncoding = Encoding.GetEncoding("windows-1250");
		/// <summary>
		/// czy logowanie w wielu liniach (normalnie) - true, lub czy kazda linia ma nadpisywac poprzdnia (false)
		/// </summary>
		private bool			m_bMultiLineLog = true;
		/// <summary>
		/// Limit rozmiaru logu (w bajtach)
		/// </summary>
		protected long			m_nFileSizeLimit = 0;
		/// <summary>
		/// Licznik linii zapisanych od ostatniego sprawdzenia rozmiaru pliku
		/// </summary>
		private int				m_nWriteLineCounter  = 0;
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
		#region Constructors
		
		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_nBufferSize"></param>
		/// <param name="i_bAppendLog"></param>
		/// <param name="i_bMuliLineLog"></param>
		/// <param name="i_sDirectory"></param>
		/// <param name="i_sLogNamePrefix"></param>
		/// <param name="i_sLogName"></param>
		/// <param name="i_sLogNameSuffix"></param>
		/// <param name="i_sLogNameExtension"></param>
		protected CTraceWriter(
			int		i_nBufferSize,
			bool	i_bAppendLog,
			bool	i_bMuliLineLog,
			string	i_sDirectory,
			string	i_sLogNamePrefix,
			string	i_sLogName,
			string	i_sLogNameSuffix,
			string	i_sLogNameExtension
		)
		{
			try
			{
				p_bEnableAssertionInFinalizer = false;	//TODO - tymczasowe rozwiazanie az do czasu wdrozenia rozwiazania wywolywania Dispose na obiektach statycznych

				string sLogDir = i_sDirectory;
				if (sLogDir == null || sLogDir=="")
					sLogDir = sm_oLogContainer.p_sDefaultLogDir;

				string sLogName = i_sLogName;
				if (sLogName == null || sLogName=="")
					sLogName = sm_oLogContainer.p_sDefaultLogName;

				CLogFileName oLogName = new CLogFileName(
					sLogDir,
					i_sLogNamePrefix,
					sLogName,
					i_sLogNameSuffix,
					i_sLogNameExtension
					);

				m_oLogName		= oLogName;
				m_sBuffer		= new string[i_nBufferSize];
				m_bMultiLineLog	= i_bMuliLineLog;

				sm_oLogContainer.AddLog(this);

				if (!i_bAppendLog)
				{
					if (File.Exists(m_oLogName.p_sLogName))
						File.Delete(m_oLogName.p_sLogName);
				}
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}

		}
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface

		/// <summary>
		/// Aktualna pozycja w buforze
		/// </summary>
		public int p_nCurrentPos
		{
			get
			{
				lock(this)
				{
					return m_nPos;
				}
			}
		}
		
		/// <summary>
		/// Encoding
		/// </summary>
		public Encoding p_oEncoding
		{
			get
			{
				return m_oEncoding;
			}
			set
			{
				CPfxAssert.IsNotNull(value);
				m_oEncoding = value;
			}
		}
		
		/// <summary>
		/// Wlasciwosc dostepowa do pola MultiLineLog
		/// </summary>
		public bool p_bMultiLineLog
		{
			get
			{
				return m_bMultiLineLog;
			}
			set
			{
				m_bMultiLineLog = value;
			}
		}

		/// <summary>
		/// zmana rozmiaru bufora, domyslnie 0(brak bufora)
		/// </summary>
		public int p_nBufferSize
		{
			get
			{
				lock(this)
				{
					return m_sBuffer.Length;
				}
			}
			set
			{
				lock(this)
				{
					try
					{
						if (m_sBuffer.Length > 0)
							FlushCore();
						m_sBuffer = new string[value];
						m_nPos = -1;
					}
					catch(Exception e)
					{
						CExceptionManager.LogException(e);
					}
				}
			}
		}

		/// <summary>
		/// Zmiana katalogu logu
		/// </summary>
		/// <param name="i_sDirectory">sciezka</param>
		public void ChangeLogDir(
			string i_sDirectory
		)
		{
			try
			{
				m_oLogName.ChangeLogDir(i_sDirectory);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// Zmiana nazwy logu
		/// </summary>
		/// <param name="i_sName">Glowny czlon nazwy logu</param>
		public void ChangeLogName(
			string i_sName
		)
		{
			try
			{
				m_oLogName.ChangeLogName(i_sName);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// Zmiana nazwy logu
		/// </summary>
		/// <param name="i_sName">Glowny czlon nazwy logu</param>
		public void AssureLogName(
			string i_sName
			)
		{
			try
			{
				m_oLogName.AssureLogName(i_sName);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// Zmiana nazwy logu
		/// </summary>
		/// <param name="i_sPrefix">Przedrostek</param>
		/// <param name="i_sName">Czesc glowna nazwy</param>
		/// <param name="i_sSuffix">Przyrostek</param>
		/// <param name="i_sExtension">Rozszerzenie</param>
		public void ChangeLogName(
			string i_sPrefix,
			string i_sName,
			string i_sSuffix,
			string i_sExtension
			)
		{
			try
			{
				m_oLogName.ChangeLogName(
					i_sPrefix,
					i_sName,
					i_sSuffix,
					i_sExtension
					);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// Zapisuje zawartosc bufora na dysk
		/// </summary>
		public void Flush()
		{
			lock(this)
			{
				try
				{
					FlushCore();
				}
				catch(Exception e)
				{
					CExceptionManager.LogException(e);
				}
			}
		} 

		/// <summary>
		/// 
		/// </summary>
		public static void FlushAll()
		{
			try
			{
				sm_oLogContainer.Flush();
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// Limit rozmiaru logu (w bajtach), 0 sprawdzanie wylaczone
		/// </summary>
		public long p_nFileSizeLimit
		{
			get
			{
				return m_nFileSizeLimit;
			}
			set
			{
				if (value >= 0)
					m_nFileSizeLimit = value;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sLogName"></param>
		public static void ChangeDefaultLogName(
			string		i_sLogName
			)
		{
			sm_oLogContainer.SetDefaultLogName(i_sLogName);
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sLogDir"></param>
		public static void ChangeDefaultLogDir(
			string		i_sLogDir
			)
		{
			sm_oLogContainer.ChangeLogDir(i_sLogDir);
			CExceptionManager.ChangeLogDir(i_sLogDir);
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sLogName"></param>
		public static void DisposeAllLogs(
		)
		{
			sm_oLogContainer.Dispose();
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
		#region PrivateInterface

		/// <summary>
		/// Zapis linii logu
		/// </summary>
		/// <param name="i_sLine">zapisywana linia</param>
		protected void Write(
			string i_sLine
		)
		{
			lock(this)
			{
				if (m_sBuffer.Length > 1)
					WriteIntoBuffer(i_sLine);
				else
					WriteIntoFile(i_sLine);
			}
		}

		/// <summary>
		/// Metoda zwalniajaca zasoby - oproznianie bufora
		/// </summary>
		/// <param name="i_bDisposing"></param>
		protected override void Dispose(
			bool i_bDisposing
		)
		{
			Flush();
			sm_oLogContainer.RemoveLog(this);
		}

		/// <summary>
		/// Zapis linii do bufora
		/// </summary>
		/// <param name="i_sItem">Zapisywana linia</param>
		/// <returns></returns>
		private int WriteIntoBuffer(
			string i_sItem
			)
		{
			m_sBuffer[++m_nPos] = i_sItem;
		
			if (m_nPos >= m_sBuffer.Length - 1)
			{
				FlushCore();
			}
			return m_nPos;
		}

		/// <summary>
		/// Oproznianie bufora
		/// </summary>
		private void FlushCore()
		{
			if (m_sBuffer == null)
				return;

			if (m_sBuffer.Length == 0)
				return;

			if (m_nPos < 0)
				return;

			if (m_nWriteLineCounter >= def_nWriteLinesBetweenSizeCheck)
				HandleFileSizeLimit();//kontrola limitu rozmiaru

			CPfxAssert.Assert(m_nPos < m_sBuffer.Length, "Aktualna pozyja poza buforem");
			using (StreamWriter oStream = new StreamWriter(
				m_oLogName.p_sLogName,
				true, 
				m_oEncoding
			))
			{
				for(int i=0; i<=m_nPos; i++)
				{
					oStream.WriteLine(m_sBuffer[i]);
				}
				
				m_nWriteLineCounter += m_nPos;
				m_nPos = -1;
			}
		}

		/// <summary>
		/// Zapis linii do pliku
		/// </summary>
		/// <param name="i_sLine"></param>
		private void WriteIntoFile(
			string i_sLine
			)
		{
			if (i_sLine == null)
				return;
			if (i_sLine.Length == 0)
				return;

			if (m_nWriteLineCounter >= def_nWriteLinesBetweenSizeCheck)
				HandleFileSizeLimit();//kontrola limitu rozmiaru

			using (StreamWriter oStream = new StreamWriter(
				m_oLogName.p_sLogName, 
				m_bMultiLineLog, 
				m_oEncoding
			))
			{
				oStream.WriteLine(i_sLine);
				oStream.Close();
				m_nWriteLineCounter++;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		private void HandleFileSizeLimit()
		{
			if (m_nFileSizeLimit == 0)
				return;

			m_nWriteLineCounter = 0;

			FileInfo oFile = new FileInfo(m_oLogName.p_sLogName);
			
			if (!oFile.Exists)
				return;

			if (oFile.Length>=m_nFileSizeLimit)
			{
				string sNewName = GetNewLogFileName(m_oLogName.p_sLogName);
				oFile.MoveTo(sNewName);
			}
		}

		private string GetNewLogFileName(
			string i_sOldName
		)
		{
			DateTime oDT = DateTime.Now;
			string sDateTime = string.Format(
				"_{0:D4}{1:D2}{2:D2}{3:D2}{4:D2}{5:D2}",
				oDT.Year,
				oDT.Month,
				oDT.Day,
				oDT.Hour,
				oDT.Minute,
				oDT.Second
			);
			int nPos = i_sOldName.Length - 4;
			return i_sOldName.Insert(nPos, sDateTime);
		}
		#endregion
	}
}
