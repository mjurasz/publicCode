using System;

namespace net.jurasz.tools
{
	/// <summary>
	/// Typy komunikatów
	/// </summary>
	[Flags]
	public enum EAppLog_MessageType // CRTASK - nazwa nie jest w liczbie mnogiej
	{
		bitTrace		= 1, // komunikat normalny
		bitWarning		= 2, // uwaga
		bitError		= 4, // blad
		bitFatalError	= 8 // blad krytyczny
	}

    ///////////////////////////////////////////////////////////////////////////////////
    // P U B L I C  I N T E R F A C E (public methods etc)
	public enum EAppLog_MessageLevel
	{
		eLow	= 0,
		eMedium = 1,
		eHigh	= 2
	}

	public interface IApplicationLogCore
	{
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
		void AppTrace(
			EAppLog_MessageType i_bitMessageType,
			EAppLog_MessageLevel i_eMessageLevel,
			string i_sGroup,
			string i_sSubGroup,
			string i_sMessage
			);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_bitMessageType"></param>
		/// <param name="i_eMessageLevel"></param>
		/// <param name="i_sGroup"></param>
		/// <param name="i_sSubGroup"></param>
		/// <param name="i_sMessage"></param>
		/// <param name="i_oParams"></param>
		void AppTrace(
			EAppLog_MessageType i_bitMessageType,
			EAppLog_MessageLevel i_eMessageLevel,
			string i_sGroup,
			string i_sSubGroup,
			string i_sMessage,
			params object[] i_oParams
			);

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

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_bitMessageType"></param>
		/// <param name="i_eMessageLevel"></param>
		/// <returns></returns>
		bool IsWriteAllowed(
			EAppLog_MessageType i_bitMessageType,
			EAppLog_MessageLevel i_eMessageLevel
			);

		#endregion
	}
}