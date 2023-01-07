using System;
using System.IO;
using System.Collections;

namespace net.jurasz.tools
{
	// /////////////////////////////////////////////////////////////////////////////////
	//								C l a s s											
	/// <summary>
	/// Klasa przechowujaca wzystkie stworzone w systemie obiekty logow
	/// </summary>
    public class CLogContainer : CDisposableBase
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
		#region Definitions
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
		#region Members

		/// <summary>
		/// kontener - lista obiektow
		/// </summary>
		private ArrayList		m_oLogList			= new ArrayList(5);
		
		/// <summary>
		/// domyslna sciezka logowania
		/// </summary>
		private string			m_sDefaultLogDir	= "Log";

		/// <summary>
		/// domyslna nazwa logow (dla logow bez nazwy)
		/// </summary>
		private string			m_sDefaultLogName	= "";

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
		#region Constructors
		public CLogContainer()
		{
			p_bEnableAssertionInFinalizer = false;
		}
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface

		/// <summary>
		/// dodaje nowy obiekt logu
		/// </summary>
		/// <param name="i_oLog"></param>
		public void AddLog(
			CTraceWriter i_oLog
		)
		{
			CPfxAssert.IsNotNull(i_oLog);
			m_oLogList.Add(i_oLog);
		}

		/// <summary>
		/// zwraca liczbe przechowywanych obiektow
		/// </summary>
		public int p_nCount
		{
			get
			{
				return m_oLogList.Count;
			}
		}

		/// <summary>
		/// zapisuje zawartosc wszystkich buforow na dysk
		/// </summary>
		public void Flush()
		{
			IEnumerator oEnum = m_oLogList.GetEnumerator();
			while (oEnum.MoveNext())
				((CTraceWriter)oEnum.Current).Flush();
		}

		/// <summary>
		/// zmienia katalog logownia 
		/// </summary>
		/// <param name="i_sLogDir">nowy katalog</param>
		public void ChangeLogDir(
			string i_sLogDir
		)
		{
			CPfxAssert.StringNotEmpty(i_sLogDir);
			
			m_sDefaultLogDir = i_sLogDir;

			if (!Directory.Exists(i_sLogDir))
				Directory.CreateDirectory(i_sLogDir);

			IEnumerator oEnum = m_oLogList.GetEnumerator();
			while (oEnum.MoveNext())
				((CTraceWriter)oEnum.Current).ChangeLogDir(i_sLogDir);
		}

		/// <summary>
		/// zmienia katalog logownia 
		/// </summary>
		/// <param name="i_sLogDir">nowy katalog</param>
		public void SetDefaultLogName(
			string i_sDefaultLogName
			)
		{
			CPfxAssert.StringNotEmpty(i_sDefaultLogName);
			
			m_sDefaultLogName = i_sDefaultLogName;

			IEnumerator oEnum = m_oLogList.GetEnumerator();
			while (oEnum.MoveNext())
				((CTraceWriter)oEnum.Current).AssureLogName(i_sDefaultLogName);
		}

		/// <summary>
		/// usuwa log z listy
		/// </summary>
		/// <param name="i_oLog"></param>
		public void RemoveLog(
			CTraceWriter	i_oLog
		)
		{
			m_oLogList.Remove(i_oLog);
		}

		/// <summary>
		/// zwraca domyslna sciezke logowania
		/// </summary>
		public string p_sDefaultLogDir
		{
			get 
			{ 
				return m_sDefaultLogDir;
			}
		}

		/// <summary>
		/// Zwraca domyslna nazwe logu
		/// </summary>
		public string p_sDefaultLogName
		{
			get
			{
				return m_sDefaultLogName;
			}
		}



		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
		#region PrivateInterface

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_bDisposing"></param>
		protected override void Dispose(
			bool i_bDisposing
			)
		{
			/// Klonowanie ze wzgledu na to, ze poszczegolne Dispose logow usuwaja sie z m_oList
			ArrayList oTempLogList = (ArrayList)m_oLogList.Clone();

			IEnumerator oEnum = oTempLogList.GetEnumerator();
			while (oEnum.MoveNext())
				((CTraceWriter)oEnum.Current).Dispose();
		}

		#endregion
	}
}
