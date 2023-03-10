using System.Diagnostics;

namespace net.jurasz.tools
{
	/// <summary>
	/// Klasa logu aplikacji (rdzeniowa). W programie powinnien istniec jeden obiekt tej klasy.
	/// Logowanie powinno byc realizowane przez klase opakowujaca (CApplicationLog)
	/// </summary>
	public class CApplicationLogCore : CTraceCore, IApplicationLogCore
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
		#region Definitions

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
		#region Members

		/// <summary>
		/// tablica konfiguracyjne logu (definicje kolumn)
		/// </summary>
		private static readonly CMessageField[] def_oMessageFieldsArray;

		/// <summary>
		/// Filter typów komunikatów
		/// </summary>
		private EAppLog_MessageType m_bitsMessageTypeFilter = EAppLog_MessageType.bitTrace | EAppLog_MessageType.bitWarning |
		                                                      EAppLog_MessageType.bitError | EAppLog_MessageType.bitFatalError;

		/// <summary>
		/// Poziom logowania
		/// </summary>
		private EAppLog_MessageLevel m_eMessageLevelFilter = EAppLog_MessageLevel.eHigh;

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
		#region Constructors

		/// <summary>
		/// Konstruktor statyczny - ustawienie definicji kolumn
		/// </summary>
		static CApplicationLogCore()
		{
			def_oMessageFieldsArray = new CMessageField[7];
			def_oMessageFieldsArray[0] = new CMessageField( EMessageFieldType.eDate );
			def_oMessageFieldsArray[1] = new CMessageField( EMessageFieldType.eTime );
			def_oMessageFieldsArray[2] = new CMessageField( EMessageFieldType.eThreadId, "Thr({0,8})" );
			def_oMessageFieldsArray[3] = new CMessageField( EMessageFieldType.eUserDefined ); // typ
			def_oMessageFieldsArray[4] = new CMessageField( EMessageFieldType.eUserDefined, "{0}", "" ); // grupa
			def_oMessageFieldsArray[5] = new CMessageField( EMessageFieldType.eUserDefined, "({0})" ); // podgrupa
			def_oMessageFieldsArray[6] = new CMessageField( EMessageFieldType.eMessage );
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sDirectory"></param>
		/// <param name="i_sLogName"></param>
		/// <param name="i_bAppendLog"></param>
		/// <param name="i_bitsMessageTypeFilter"></param>
		/// <param name="i_eMessageLevelFilter"></param>
		/// <param name="i_nMaxMessageWidth"></param>
		public CApplicationLogCore(
			string i_sDirectory,
			string i_sLogName,
			bool i_bAppendLog,
			EAppLog_MessageType i_bitsMessageTypeFilter,
			EAppLog_MessageLevel i_eMessageLevelFilter,
			int i_nMaxMessageWidth
			) : base(
				i_sDirectory,
				"APL",
				i_sLogName,
				null,
				null,
				i_nMaxMessageWidth,
				def_oMessageFieldsArray,
				i_bAppendLog,
				1
				)
		{
			m_bitsMessageTypeFilter = i_bitsMessageTypeFilter;
			m_eMessageLevelFilter = i_eMessageLevelFilter;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sLogName"></param>
		/// <param name="i_bAppendLog"></param>
		/// <param name="i_bitsMessageTypeFilter"></param>
		/// <param name="i_eMessageLevelFilter"></param>
		/// <param name="i_nMaxMessageWidth"></param>
		public CApplicationLogCore(
			string i_sLogName,
			bool i_bAppendLog,
			EAppLog_MessageType i_bitsMessageTypeFilter,
			EAppLog_MessageLevel i_eMessageLevelFilter,
			int i_nMaxMessageWidth
			) : this(
				null,
				i_sLogName,
				i_bAppendLog,
				i_bitsMessageTypeFilter,
				i_eMessageLevelFilter,
				i_nMaxMessageWidth
				)
		{
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sLogName"></param>
		/// <param name="i_bAppendLog"></param>
		public CApplicationLogCore(
			string i_sLogName,
			bool i_bAppendLog
			) : this(
				null,
				i_sLogName,
				i_bAppendLog,
				EAppLog_MessageType.bitTrace | EAppLog_MessageType.bitWarning | EAppLog_MessageType.bitError |
				EAppLog_MessageType.bitFatalError,
				EAppLog_MessageLevel.eHigh,
				0
				)
		{
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_bitMessageType"></param>
		/// <param name="i_eMessageLevel"></param>
		/// <param name="i_sGroup"></param>
		/// <param name="i_sSubGroup"></param>
		/// <param name="i_sMessage"></param>
		public void AppTrace(
			EAppLog_MessageType i_bitMessageType,
			EAppLog_MessageLevel i_eMessageLevel,
			string i_sGroup,
			string i_sSubGroup,
			string i_sMessage
			)
		{
			if (!IsWriteAllowed( i_bitMessageType, i_eMessageLevel ))
			{
				return;
			}

			string sMsgType = GetMessageType(
				i_bitMessageType,
				i_eMessageLevel
				);

			object[] oUserDefined = new object[] {sMsgType, i_sGroup, i_sSubGroup};
			Trace(
				oUserDefined,
				i_sMessage
				);

			if (p_nBufferSize > 1)
			{
				if (( i_bitMessageType & EAppLog_MessageType.bitFatalError ) == EAppLog_MessageType.bitFatalError ||
				    ( i_bitMessageType & EAppLog_MessageType.bitError ) == EAppLog_MessageType.bitError ||
				    ( i_bitMessageType & EAppLog_MessageType.bitWarning ) == EAppLog_MessageType.bitWarning ||
				    ( ( ( i_bitMessageType & EAppLog_MessageType.bitTrace ) == EAppLog_MessageType.bitTrace ) &&
				      ( i_eMessageLevel == EAppLog_MessageLevel.eLow ) )
					)
				{
					Flush();
				}
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_bitMessageType"></param>
		/// <param name="i_eMessageLevel"></param>
		/// <param name="i_sGroup"></param>
		/// <param name="i_sSubGroup"></param>
		/// <param name="i_sMessage"></param>
		/// <param name="i_oParams"></param>
		public void AppTrace(
			EAppLog_MessageType i_bitMessageType,
			EAppLog_MessageLevel i_eMessageLevel,
			string i_sGroup,
			string i_sSubGroup,
			string i_sMessage,
			params object[] i_oParams
			)
		{
			if (!IsWriteAllowed( i_bitMessageType, i_eMessageLevel ))
			{
				return;
			}

			string sMsgType = GetMessageType(
				i_bitMessageType,
				i_eMessageLevel
				);

			object[] oUserDefined = new object[] {sMsgType, i_sGroup, i_sSubGroup};
			Trace(
				oUserDefined,
				i_sMessage,
				i_oParams
				);

			if (p_nBufferSize > 1)
			{
				if (( i_bitMessageType & EAppLog_MessageType.bitFatalError ) == EAppLog_MessageType.bitFatalError ||
				    ( i_bitMessageType & EAppLog_MessageType.bitError ) == EAppLog_MessageType.bitError ||
				    ( i_bitMessageType & EAppLog_MessageType.bitWarning ) == EAppLog_MessageType.bitWarning ||
				    ( ( ( i_bitMessageType & EAppLog_MessageType.bitTrace ) == EAppLog_MessageType.bitTrace ) &&
				      ( i_eMessageLevel == EAppLog_MessageLevel.eLow ) )
					)
				{
					Flush();
				}
			}
		}

        public void Flush() { base.Flush(); }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="i_bWriteImmediately"></param>
        public void Flush(
            bool i_bWriteImmediately
            ) { base.Flush(); }

		/// <summary>
		/// 
		/// </summary>
		public EAppLog_MessageType p_eMessageType
		{
			get
			{
				return m_bitsMessageTypeFilter;
			}
			set
			{
				m_bitsMessageTypeFilter = value;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		public EAppLog_MessageLevel p_eMessageLevel
		{
			get
			{
				return m_eMessageLevelFilter;
			}
			set
			{
				m_eMessageLevelFilter = value;
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_eMessageType"></param>
		public void EnableMessageType(
			EAppLog_MessageType i_eMessageType
			)
		{
			m_bitsMessageTypeFilter |= i_eMessageType;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_eMessageType"></param>
		public void DisableMessageType(
			EAppLog_MessageType i_eMessageType
			)
		{
			m_bitsMessageTypeFilter &= ~i_eMessageType;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_bitMessageType"></param>
		/// <param name="i_eMessageLevel"></param>
		/// <returns></returns>
		public bool IsWriteAllowed(
			EAppLog_MessageType i_bitMessageType,
			EAppLog_MessageLevel i_eMessageLevel
			)
		{
			if (!p_bLogOn) // log wylaczony
			{
				return false;
			}

			if (( i_bitMessageType & m_bitsMessageTypeFilter ) == 0) // komunikat typu nie logowanego
			{
				return false;
			}

			if (i_bitMessageType == EAppLog_MessageType.bitTrace)
			{
				if (i_eMessageLevel > m_eMessageLevelFilter) // komunikat o zbyt wysokim poziomie
				{
					return false;
				}
			}

			return true;
		}

    	#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)	
		#region PrivateInterface

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // I N T E R N A L  F U N C T I O N S
		#region InternalFunctions

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_bitMessageType"></param>
		/// <param name="i_eMessageLevel"></param>
		/// <returns></returns>
		private static string GetMessageType(
			EAppLog_MessageType i_bitMessageType,
			EAppLog_MessageLevel i_eMessageLevel
			)
		{
			switch (i_bitMessageType)
			{
				case EAppLog_MessageType.bitTrace:
					{
						switch (i_eMessageLevel)
						{
							case EAppLog_MessageLevel.eLow:
								return "LOW";
							case EAppLog_MessageLevel.eMedium:
								return "MEDIUM";
							case EAppLog_MessageLevel.eHigh:
								return "HIGH";
							default:
								Debug.Assert( false );
								return "UNKNOWN TRACE";
						}
					}
				case EAppLog_MessageType.bitWarning:
					return "WARNING";

				case EAppLog_MessageType.bitError:
					return "ERROR";

				case EAppLog_MessageType.bitFatalError:
					return "FATAL ERROR";

				default:
					Debug.Assert( false );
					return "UNKNOWN TYPE";
			}
		}

		#endregion
	}
}