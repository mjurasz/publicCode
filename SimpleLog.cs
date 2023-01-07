namespace net.jurasz.tools
{
	// /////////////////////////////////////////////////////////////////////////////////
	//								C l a s s											
	/// <summary>
	/// Klasa logu developerskiego
	/// </summary>
	public class CSimpleLog : CTraceCore
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
		#region Definitions

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
		#region Members

		private static readonly CMessageField[] def_oMessageFieldsArray;

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
		#region Constructors

		/// <summary>
		/// 
		/// </summary>
		static CSimpleLog()
		{
			def_oMessageFieldsArray = new CMessageField[4];
			def_oMessageFieldsArray[0] = new CMessageField( EMessageFieldType.eDate );
			def_oMessageFieldsArray[1] = new CMessageField( EMessageFieldType.eTime );
			def_oMessageFieldsArray[2] = new CMessageField( EMessageFieldType.eThreadId, "Thr({0,8})" );
			def_oMessageFieldsArray[3] = new CMessageField( EMessageFieldType.eMessage );
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sDirectory"></param>
		/// <param name="i_sLogNamePrefix"></param>
		/// <param name="i_sLogName"></param>
		/// <param name="i_bAppendLog"></param>
		/// <param name="i_nBufferSize"></param>
		public CSimpleLog(
			string i_sDirectory,
			string i_sLogNamePrefix,
			string i_sLogName,
			bool i_bAppendLog,
			int i_nBufferSize
			) : base(
				i_sDirectory,
				i_sLogNamePrefix,
				i_sLogName,
				null,
				null,
				0,
				def_oMessageFieldsArray,
				i_bAppendLog,
				i_nBufferSize
				)
		{
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sLogNamePrefix"></param>
		/// <param name="i_sLogName"></param>
		/// <param name="i_bAppendLog"></param>
		/// <param name="i_nBufferSize"></param>
		public CSimpleLog(
			string i_sLogNamePrefix,
			string i_sLogName,
			bool i_bAppendLog,
			int i_nBufferSize
			) : this(
				null,
				i_sLogNamePrefix,
				i_sLogName,
				i_bAppendLog,
				i_nBufferSize
				)
		{
			// konstruktor przekierowany
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sDirectory"></param>
		/// <param name="i_sLogNamePrefix"></param>
		/// <param name="i_sLogName"></param>
		/// <param name="i_bAppendLog"></param>
		/// <param name="i_nBufferSize"></param>
		/// <param name="i_bSingleTraceLog"></param>
		public CSimpleLog(
			string i_sDirectory,
			string i_sLogNamePrefix,
			string i_sLogName,
			bool i_bAppendLog,
			int i_nBufferSize,
			bool i_bSingleTraceLog
			) : base(
				i_sDirectory,
				i_sLogNamePrefix,
				i_sLogName,
				null,
				null,
				0,
				def_oMessageFieldsArray,
				i_bAppendLog,
				i_nBufferSize,
				!i_bSingleTraceLog
				)
		{
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sLogNamePrefix"></param>
		/// <param name="i_sLogName"></param>
		/// <param name="i_bAppendLog"></param>
		/// <param name="i_nBufferSize"></param>
		/// <param name="i_bSingleTraceLog"></param>
		public CSimpleLog(
			string i_sLogNamePrefix,
			string i_sLogName,
			bool i_bAppendLog,
			int i_nBufferSize,
			bool i_bSingleTraceLog
			) : this(
				null,
				i_sLogNamePrefix,
				i_sLogName,
				i_bAppendLog,
				i_nBufferSize,
				i_bSingleTraceLog
				)
		{
			// konstruktor przekierowany
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
		#region InternalFunctions

		#endregion
	}
}