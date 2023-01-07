using System;
using System.Diagnostics;

namespace net.jurasz.tools
{
	// /////////////////////////////////////////////////////////////////////////////////
	//								C l a s s											
	/// <summary>
	/// Klasa logu developerskiego
	/// </summary>
	public class CDevelopTrace : CSimpleLog
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
		#region Definitions

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
		#region Members

		private int			m_nMaxTraceLevel;
		
		public static bool	sm_bGlobalLogOn = true; // CRTASK - public

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
		#region Constructors

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sDirectory"></param>
		/// <param name="i_sLogName"></param>
		/// <param name="i_bAppendLog"></param>
		/// <param name="i_nBufferSize"></param>
		/// <param name="i_nMaxTraceLevel"></param>
		public CDevelopTrace(
			string i_sDirectory,
			string i_sLogName,
			bool i_bAppendLog,
			int i_nBufferSize,
			int i_nMaxTraceLevel
			) : base(
				i_sDirectory,
				"DTR",
				i_sLogName,
				i_bAppendLog,
				i_nBufferSize
				)
		{
			m_nMaxTraceLevel = i_nMaxTraceLevel;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sLogName"></param>
		/// <param name="i_bAppendLog"></param>
		/// <param name="i_nBufferSize"></param>
		/// <param name="i_nMaxTraceLevel"></param>
		public CDevelopTrace(
			string i_sLogName,
			bool i_bAppendLog,
			int i_nBufferSize,
			int i_nMaxTraceLevel
			) : base(
				"DTR",
				i_sLogName,
				i_bAppendLog,
				i_nBufferSize
				)
		{
			m_nMaxTraceLevel = i_nMaxTraceLevel;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sDirectory"></param>
		/// <param name="i_sLogName"></param>
		/// <param name="i_bAppendLog"></param>
		/// <param name="i_nBufferSize"></param>
		/// <param name="i_nMaxTraceLevel"></param>
		/// <param name="i_bSingleTraceLog"></param>
		public CDevelopTrace(
			string i_sDirectory,
			string i_sLogName,
			bool i_bAppendLog,
			int i_nBufferSize,
			int i_nMaxTraceLevel,
			bool i_bSingleTraceLog
			) : base(
				i_sDirectory,
				"DTR",
				i_sLogName,
				i_bAppendLog,
				i_nBufferSize,
				i_bSingleTraceLog
				)
		{
			m_nMaxTraceLevel = i_nMaxTraceLevel;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sLogName"></param>
		/// <param name="i_bAppendLog"></param>
		/// <param name="i_nBufferSize"></param>
		/// <param name="i_nMaxTraceLevel"></param>
		/// <param name="i_bSingleTraceLog"></param>
		public CDevelopTrace(
			string i_sLogName,
			bool i_bAppendLog,
			int i_nBufferSize,
			int i_nMaxTraceLevel,
			bool i_bSingleTraceLog
			) : base(
				"DTR",
				i_sLogName,
				i_bAppendLog,
				i_nBufferSize,
				i_bSingleTraceLog
				)
		{
			m_nMaxTraceLevel = i_nMaxTraceLevel;
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_nTraceLevel"></param>
		/// <param name="i_sMessage"></param>
		[Conditional( "DEBUG" ), Conditional( "UFXUSEDEVTRACE" )]
		public void Log(
			int i_nTraceLevel,
			string i_sMessage
			)
		{
			try
			{
				if (!sm_bGlobalLogOn)
				{
					return;
				}

				if (i_nTraceLevel > m_nMaxTraceLevel)
				{
					return;
				}
				Trace( i_sMessage );
			}
			catch (Exception e)
			{
				CExceptionManager.LogException( e );
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_nTraceLevel"></param>
		/// <param name="i_sMessage"></param>
		/// <param name="i_oParams"></param>
		[Conditional( "DEBUG" ), Conditional( "UFXUSEDEVTRACE" )]
		public void Log(
			int i_nTraceLevel,
			string i_sMessage,
			params object[] i_oParams
			)
		{
			try
			{
				if (!sm_bGlobalLogOn)
				{
					return;
				}

				if (i_nTraceLevel > m_nMaxTraceLevel)
				{
					return;
				}

				Trace( i_sMessage, i_oParams );
			}
			catch (Exception e)
			{
				CExceptionManager.LogException( e );
			}
		}

		/// <summary>
		/// 
		/// </summary>
		public int p_nMaxTraceLevel
		{
			get
			{
				return m_nMaxTraceLevel;
			}

			set
			{
				m_nMaxTraceLevel = value;
			}
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
		#region InternalFunctions

		#endregion
	}
}