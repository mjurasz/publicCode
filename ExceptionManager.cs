using System;
using System.IO;
using System.Text;
using System.Diagnostics;
using System.Reflection;

namespace net.jurasz.tools
{
	// /////////////////////////////////////////////////////////////////////////////////
	//								C l a s s											
	/// <summary>
	/// 
	/// </summary>
	public class CExceptionManager
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
		#region Definitions
		
		#endregion

		///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
		#region Members
		/// <summary>
		/// 
		/// </summary>
		private static string		sm_sFileName;
		/// <summary>
		/// 
		/// </summary>
		private static Encoding		sm_oEncoding = Encoding.GetEncoding("windows-1250");
		/// <summary>
		/// 
		/// </summary>
		private static string		sm_sLogDir = AppDomain.CurrentDomain.BaseDirectory + "Logs";
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)						
		#region Constructors

		/// <summary>
		/// 
		/// </summary>
		static CExceptionManager()
		{
            // sm_sFileName = sm_sLogDir + "\\" + "Exceptions_" + AppDomain.CurrentDomain.FriendlyName.Replace("exe","log");
			sm_sFileName = sm_sLogDir + "\\" + "Exceptions.log";
		}
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface

		/// <summary>
		/// 
		/// </summary>
		/// <param name="e"></param>
		public static void LogException(
			Exception	e
		)
		{
			try
			{
				using (StreamWriter oWriteStream = new StreamWriter(
						   sm_sFileName, 
						   true, 
						   sm_oEncoding
						   ))
				{
					bool bIsInner = false;
					while(e != null)
					{
						oWriteStream.WriteLine(FormatLine(e, bIsInner));
						bIsInner = true;
						e = e.InnerException;
					}
					oWriteStream.WriteLine("\n\n######################################################################\n");
					oWriteStream.Close();
				}
			}
			catch
			{				
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="i_sLogDir"></param>
		public static void ChangeLogDir(
			string i_sLogDir
			)
		{
			string sLogDir = i_sLogDir;

			if (i_sLogDir[i_sLogDir.Length - 1] !=  '\\')
				sLogDir = i_sLogDir + "\\";

            //sm_sFileName = sLogDir + "Exceptions_" + AppDomain.CurrentDomain.FriendlyName.Replace("exe","log");
            sm_sFileName = sLogDir + "Exceptions.log";
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
		#region PrivateInterface

		/// <summary>
		/// 
		/// </summary>
		/// <param name="e"></param>
		/// <param name="i_bIsInner"></param>
		/// <returns></returns>
		private static string FormatLine(
			Exception	e,
			bool		i_bIsInner
		)
		{
			DateTime oDT = DateTime.Now;

			string sLine = string.Format(
				"{0}.{1}\tThr({2})\t{3}{4}({5})\n{6}{7}",
				oDT.ToString(),
				oDT.Millisecond.ToString(),
				System.Threading.Thread.CurrentThread.ManagedThreadId.ToString(),
				(i_bIsInner ? "(Inner)" : ""),
				e.GetType().Name,
				e.Message,
				(i_bIsInner ? "" : GetEnvironmentStack()),
				GetExceptionStack(e)
			);
			return sLine;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		private static string GetEnvironmentStack()
		{
			StackTrace oStack = new StackTrace(3, true);

			StringBuilder	oMessage	= new StringBuilder(100);
			StackFrame		oFrame		= null;
			MethodBase		oMethod		= null;

			oMessage.Append("environment stack trace:\n");

			for (int i = 0; i<oStack.FrameCount ; i++)
			{
				oFrame	= oStack.GetFrame(i);
				oMethod = oFrame.GetMethod();
				oMessage.AppendFormat("\t{0}.{1}", oMethod.DeclaringType.FullName, oFrame.ToString());
			}

			return oMessage.ToString();
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="e"></param>
		/// <returns></returns>
		private static string GetExceptionStack(
			Exception	e
		)
		{
			if (e.StackTrace==null)
				return "exception stack trace:(null)";
			return "exception stack trace:\n" + e.StackTrace;
		}
		#endregion
	}
}
