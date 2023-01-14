using System;
using System.Collections;
using System.Resources;
using System.Reflection;

namespace net.jurasz.tools
{
	///<summary>
	///
	///</summary>
	public interface IApplicationLog
	{
		/// <summary>
		/// Logowanie bledu krytycznego
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lanucha z zasobów</param>
		void LogFatalError(
			string i_sMessage
			);

		/// <summary>
		/// Logowanie bledu krytycznego
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		void LogFatalError(
			string i_sMessage,
			params object[] i_oParams
			);

		/// <summary>
		/// Logowanie bledu 
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		void LogError(
			string i_sMessage
			);

		/// <summary>
		/// Logowanie bledu 
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		void LogError(
			string i_sMessage,
			params object[] i_oParams
			);

		/// <summary>
		/// Logowanie ostrze¿enia
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		void LogWarning(
			string i_sMessage
			);

		/// <summary>
		/// Logowanie ostrze¿enia
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		void LogWarning(
			string i_sMessage,
			params object[] i_oParams
			);

		/// <summary>
		/// Logowanie komunikatu niskiego poziomu
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		void LogLow(
			string i_sMessage
			);

		/// <summary>
		/// Logowanie komunikatu niskiego poziomu
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		void LogLow(
			string i_sMessage,
			params object[] i_oParams
			);

		/// <summary>
		/// Logowanie komunikatu sredniego poziomu
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		void LogMedium(
			string i_sMessage
			);

		/// <summary>
		/// Logowanie komunikatu sredniego poziomu
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		void LogMedium(
			string i_sMessage,
			params object[] i_oParams
			);

		/// <summary>
		/// Logowanie komunikatu wysokiego poziomu
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		void LogHigh(
			string i_sMessage
			);

		/// <summary>
		/// Logowanie komunikatu wysokiego poziomu
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		void LogHigh(
			string i_sMessage,
			params object[] i_oParams
			);

		/// <summary>
		/// Zwraca referencje logu wlasciwego
		/// </summary>
		IApplicationLogCore p_oLogCore { get; }

		/// <summary>
		/// 
		/// </summary>
		void Flush();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_bWriteImmediately"></param>
		void Flush(
			bool i_bWriteImmediately
			);
	}

	/// <summary>
	/// Klasa koncowa logu aplikacji. W zale¿nosci od obecnosci obiektu obslugi zasobów, 
	/// komunikaty sa brane albo bezposrednio z kodu albo z zasobów.
	/// </summary>
	public class CApplicationLog : IApplicationLog
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
        #region Definitions

		/// <summary>
		/// Klasa opakowujaca pobieranie lancucha z zasobów
		/// </summary>
		private class CAppResourceManager : CResourceManagerEx
		{
			/// <summary>
			/// 
			/// </summary>
			/// <param name="i_sBaseName"></param>
			/// <param name="i_oAssembly"></param>
			public CAppResourceManager(
				string i_sBaseName,
				Assembly i_oAssembly
				) : base(
					new ResourceManager(
						i_sBaseName,
						i_oAssembly
						),
					ENoMessageAction.eReturnUserString
					)
			{
			}

			/// <summary>
			/// 
			/// </summary>
			/// <param name="i_oResourceManager"></param>
			public CAppResourceManager(
				ResourceManager i_oResourceManager
				) : base(
					i_oResourceManager,
					ENoMessageAction.eReturnUserString
					)
			{
			}

			/// <summary>
			/// 
			/// </summary>
			/// <param name="i_oResourceManager"></param>
			public CAppResourceManager(
				CResourceManagerEx i_oResourceManager
				) : base(
					i_oResourceManager
					)
			{
			}

			/// <summary>
			/// 
			/// </summary>
			/// <param name="i_sMessageId"></param>
			/// <returns></returns>
			protected override string UserEmptyMessage(
				string i_sMessageId
				)
			{
				CPfxAssert.StringNotEmpty( i_sMessageId );
                if (string.IsNullOrEmpty(i_sMessageId))
				{
					return "Resource string not found - No String Id";
				}
				return string.Format( "Resource string not found ({0})", i_sMessageId );
			}
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
		#region Members

		private readonly IApplicationLogCore	m_oAppLog;				// docelowy log aplikacji
		private readonly string					m_sGroupName;			// nazwa grupy
		private readonly string					m_sSubGroupName;		// nazwa podgrupy
		private readonly CAppResourceManager	m_oResourceManager;		// obiekt obslugi zasobów

		private Hashtable						m_htResourceCache;		// opcjonalny cache danych z zasobów

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
		#region Constructors

		/// <summary>
		/// Konstruktor tworzacy obiekt logu korzystajacy z zasobów
		/// </summary>
		/// <param name="i_sBaseName">Nazwa pliku zasobu</param>
		/// <param name="i_oAssembly">Assembly w którym jest zasób</param>
		/// <param name="i_oAppLog">Referencja wlasciwego obiektu logu</param>
		/// <param name="i_sGroupName">Nazwa grupy</param>
		/// <param name="i_sSubGroupName">Nazwa podgrupy</param>
		public CApplicationLog(
			string				i_sBaseName,
			Assembly			i_oAssembly,
			IApplicationLogCore i_oAppLog,
			string 				i_sGroupName,
			string 				i_sSubGroupName
			)
		{
			CPfxAssert.StringNotEmpty( i_sBaseName );
			CPfxAssert.IsNotNull( i_oAssembly );
			CPfxAssert.IsNotNull( i_oAppLog );
			CPfxAssert.IsNotNull( i_sGroupName );
			CPfxAssert.IsNotNull( i_sSubGroupName );

			m_oResourceManager	= new CAppResourceManager( i_sBaseName, i_oAssembly );
			m_oAppLog			= i_oAppLog;
			m_sGroupName		= i_sGroupName;
			m_sSubGroupName		= i_sSubGroupName;
		}

		/// <summary>
		/// Konstruktor tworzacy obiekt logu korzystajacy z zasobów
		/// </summary>
		/// <param name="i_oResourceManager">referencja do obiektu obslugi zasobów</param>
		/// <param name="i_oAppLog">Referencja wlasciwego obiektu logu</param>
		/// <param name="i_sGroupName">Nazwa grupy</param>
		/// <param name="i_sSubGroupName">Nazwa podgrupy</param>
		public CApplicationLog(
			ResourceManager		i_oResourceManager,
			IApplicationLogCore i_oAppLog,
			string				i_sGroupName,
			string				i_sSubGroupName
			)
		{
			CPfxAssert.IsNotNull( i_oAppLog );
			CPfxAssert.IsNotNull( i_sGroupName );
			CPfxAssert.IsNotNull( i_sSubGroupName );

			m_oResourceManager	= new CAppResourceManager( i_oResourceManager );
			m_oAppLog			= i_oAppLog;
			m_sGroupName		= i_sGroupName;
			m_sSubGroupName		= i_sSubGroupName;
		}

		/// <summary>
		/// Konstruktor tworzacy obiekt logu nie korzystajacy z zasobów
		/// </summary>
		/// <param name="i_oAppLog">Wlasciwy obiekt logu</param>
		/// <param name="i_sGroupName">Nazwa grupy</param>
		/// <param name="i_sSubGroupName">Nazwa podgrupy</param>
		public CApplicationLog(
			IApplicationLogCore i_oAppLog,
			string 				i_sGroupName,
			string 				i_sSubGroupName
			)
		{
			CPfxAssert.IsNotNull( i_oAppLog );
			CPfxAssert.IsNotNull( i_sGroupName );
			CPfxAssert.IsNotNull( i_sSubGroupName );

			m_oAppLog		= i_oAppLog;
			m_sGroupName	= i_sGroupName;
			m_sSubGroupName = i_sSubGroupName;
		}

		/// <summary>
		/// Konstruktor tworzacy obiekt logu kopiujacy log wewnetrzny i obiekt zasobów
		/// </summary>
		/// <param name="i_oAppLog"></param>
		/// <param name="i_sGroupName"></param>
		/// <param name="i_sSubGroupName"></param>
		public CApplicationLog(
			CApplicationLog i_oAppLog,
			string 			i_sGroupName,
			string 			i_sSubGroupName
			)
		{
			CPfxAssert.IsNotNull( i_oAppLog );
			CPfxAssert.IsNotNull( i_sGroupName );
			CPfxAssert.IsNotNull( i_sSubGroupName );

			m_oAppLog = i_oAppLog.p_oLogCore;
			if (i_oAppLog.m_oResourceManager != null)
			{
				m_oResourceManager = new CAppResourceManager( i_oAppLog.m_oResourceManager );
			}
			m_sGroupName = i_sGroupName;
			m_sSubGroupName = i_sSubGroupName;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_htResourceCache"></param>
		public void Ctor_SetResourceCache(
			Hashtable i_htResourceCache // mo¿e byc null
			)
		{
			m_htResourceCache = i_htResourceCache;
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface

		/// <summary>
		/// Logowanie bledu krytycznego
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lanucha z zasobów</param>
		public void LogFatalError(
			string i_sMessage
			)
		{
			try
			{
				TracePrv(
					EAppLog_MessageType.bitFatalError,
					i_sMessage
					);
			}
			catch (Exception e)
			{
				CExceptionManager.LogException( e );
			}
		}

		/// <summary>
		/// Logowanie bledu krytycznego
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		public void LogFatalError(
			string i_sMessage,
			params object[] i_oParams
			)
		{
			try
			{
				TracePrv(
					EAppLog_MessageType.bitFatalError,
					i_sMessage,
					i_oParams
					);
			}
			catch (Exception e)
			{
                CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// Logowanie bledu 
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		public void LogError(
			string i_sMessage
			)
		{
			try
			{
				TracePrv(
					EAppLog_MessageType.bitError,
					i_sMessage
					);
			}
			catch (Exception e)
			{
				CExceptionManager.LogException( e );
			}
		}

		/// <summary>
		/// Logowanie bledu 
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		public void LogError(
			string i_sMessage,
			params object[] i_oParams
			)
		{
			try
			{
				TracePrv(
					EAppLog_MessageType.bitError,
					i_sMessage,
					i_oParams
					);
			}
			catch (Exception e)
			{
				CExceptionManager.LogException( e );
			}
		}

		/// <summary>
		/// Logowanie ostrze¿enia
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		public void LogWarning(
			string i_sMessage
			)
		{
			try
			{
				TracePrv(
					EAppLog_MessageType.bitWarning,
					i_sMessage
					);
			}
			catch (Exception e)
			{
				CExceptionManager.LogException( e );
			}
		}

		/// <summary>
		/// Logowanie ostrze¿enia
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		public void LogWarning(
			string i_sMessage,
			params object[] i_oParams
			)
		{
			try
			{
				TracePrv(
					EAppLog_MessageType.bitWarning,
					i_sMessage,
					i_oParams
					);
			}
			catch (Exception e)
			{
				CExceptionManager.LogException( e );
			}
		}

		/// <summary>
		/// Logowanie komunikatu niskiego poziomu
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		public void LogLow(
			string i_sMessage
			)
		{
			try
			{
				TracePrv(
					EAppLog_MessageLevel.eLow,
					i_sMessage
					);
			}
			catch (Exception e)
			{
				CExceptionManager.LogException( e );
			}
		}

		/// <summary>
		/// Logowanie komunikatu niskiego poziomu
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		public void LogLow(
			string i_sMessage,
			params object[] i_oParams
			)
		{
			try
			{
				TracePrv(
					EAppLog_MessageLevel.eLow,
					i_sMessage,
					i_oParams
					);
			}
			catch (Exception e)
			{
				CExceptionManager.LogException( e );
			}
		}


		/// <summary>
		/// Logowanie komunikatu sredniego poziomu
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		public void LogMedium(
			string i_sMessage
			)
		{
			try
			{
				TracePrv(
					EAppLog_MessageLevel.eMedium,
					i_sMessage
					);
			}
			catch (Exception e)
			{
				CExceptionManager.LogException( e );
			}
		}

		/// <summary>
		/// Logowanie komunikatu sredniego poziomu
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		public void LogMedium(
			string i_sMessage,
			params object[] i_oParams
			)
		{
			try
			{
				TracePrv(
					EAppLog_MessageLevel.eMedium,
					i_sMessage,
					i_oParams
					);
			}
			catch (Exception e)
			{
				CExceptionManager.LogException( e );
			}
		}

		/// <summary>
		/// Logowanie komunikatu wysokiego poziomu
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		public void LogHigh(
			string i_sMessage
			)
		{
			try
			{
				TracePrv(
					EAppLog_MessageLevel.eHigh,
					i_sMessage
					);
			}
			catch (Exception e)
			{
				CExceptionManager.LogException( e );
			}
		}

		/// <summary>
		/// Logowanie komunikatu wysokiego poziomu
		/// </summary>
		/// <param name="i_sMessage">Komunikat lub identyfikator lancucha z zasobów</param>
		/// <param name="i_oParams">Parametry komunikatu</param>
		public void LogHigh(
			string i_sMessage,
			params object[] i_oParams
			)
		{
			try
			{
				TracePrv(
					EAppLog_MessageLevel.eHigh,
					i_sMessage,
					i_oParams
					);
			}
// ReSharper disable EmptyGeneralCatchClause
			catch (Exception)
// ReSharper restore EmptyGeneralCatchClause
			{
			}
		}

		/// <summary>
		/// Zwraca referencje logu wlasciwego
		/// </summary>
		public IApplicationLogCore p_oLogCore
		{
			get
			{
				return m_oAppLog;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		public Hashtable p_htResourceCache
		{
			get
			{
				return m_htResourceCache;
			}
		}

		//		/// <summary>
		//		/// 
		//		/// </summary>
		//		public ResourceManager p_oResourceManager
		//		{
		//			get
		//			{
		//				return m_oResourceManager;
		//			}
		//		}

		/// <summary>
		/// 
		/// </summary>
		public void Flush()
		{
			m_oAppLog.Flush();
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_bWriteImmediately"></param>
		public void Flush(
			bool i_bWriteImmediately
			)
		{
			m_oAppLog.Flush( i_bWriteImmediately );
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // I N T E R N A L  F U N C T I O N S
		#region InternalFunctions

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_eMsgType"></param>
		/// <param name="i_sMessage"></param>
		private void TracePrv(
			EAppLog_MessageType i_eMsgType,
			string i_sMessage
			)
		{
			if (m_oAppLog == null)
			{
				return;
			}

			if (!m_oAppLog.IsWriteAllowed( i_eMsgType, EAppLog_MessageLevel.eLow ))
			{
				return;
			}

			string sResourceString = GetResourceString( i_sMessage );

			m_oAppLog.AppTrace(
				i_eMsgType,
				EAppLog_MessageLevel.eLow,
				m_sGroupName,
				m_sSubGroupName,
				sResourceString
				);
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_eMsgType"></param>
		/// <param name="i_sMessage"></param>
		/// <param name="i_oParams"></param>
		private void TracePrv(
			EAppLog_MessageType i_eMsgType,
			string i_sMessage,
			params object[] i_oParams
			)
		{
			if (m_oAppLog == null)
			{
				return;
			}

			if (!m_oAppLog.IsWriteAllowed( i_eMsgType, EAppLog_MessageLevel.eLow ))
			{
				return;
			}

			string sResourceString = GetResourceString( i_sMessage );

			m_oAppLog.AppTrace(
				i_eMsgType,
				EAppLog_MessageLevel.eLow,
				m_sGroupName,
				m_sSubGroupName,
				sResourceString,
				i_oParams
				);
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_eMsgLevel"></param>
		/// <param name="i_sMessage"></param>
		private void TracePrv(
			EAppLog_MessageLevel i_eMsgLevel,
			string i_sMessage
			)
		{
			if (m_oAppLog == null)
			{
				return;
			}

			if (!m_oAppLog.IsWriteAllowed( EAppLog_MessageType.bitTrace, i_eMsgLevel ))
			{
				return;
			}

			string sResourceString = GetResourceString( i_sMessage );

			m_oAppLog.AppTrace(
				EAppLog_MessageType.bitTrace,
				i_eMsgLevel,
				m_sGroupName,
				m_sSubGroupName,
				sResourceString
				);
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_eMsgLevel"></param>
		/// <param name="i_sMessage"></param>
		/// <param name="i_oParams"></param>
		private void TracePrv(
			EAppLog_MessageLevel i_eMsgLevel,
			string i_sMessage,
			params object[] i_oParams
			)
		{
			if (m_oAppLog == null)
			{
				return;
			}

			if (!m_oAppLog.IsWriteAllowed( EAppLog_MessageType.bitTrace, i_eMsgLevel ))
			{
				return;
			}

			string sResourceString = GetResourceString( i_sMessage );
			m_oAppLog.AppTrace(
				EAppLog_MessageType.bitTrace,
				i_eMsgLevel,
				m_sGroupName,
				m_sSubGroupName,
				sResourceString,
				i_oParams
				);
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sMessage"></param>
		/// <returns></returns>
		private string GetResourceString(
			string i_sMessage
			)
		{
			if (m_oResourceManager == null)
			{
				return i_sMessage;
			}

			if (m_htResourceCache != null)
			{
				if (m_htResourceCache.ContainsKey( i_sMessage ))
				{
					return (string) m_htResourceCache[i_sMessage];
				}

				string sMessage = m_oResourceManager.GetResourceString( i_sMessage );
				lock (m_htResourceCache)
				{
					if (!m_htResourceCache.ContainsKey( i_sMessage ))
					{
						m_htResourceCache.Add( i_sMessage, sMessage );
					}
				}
				return sMessage;
			}

			return m_oResourceManager.GetResourceString( i_sMessage );
		}

		#endregion
	}
}