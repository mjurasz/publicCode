using System;
using System.Diagnostics;
using System.Reflection;

namespace net.jurasz.tools
{
	// /////////////////////////////////////////////////////////////////////////////////
	//								C l a s s											
	/// <summary>
	/// </summary>
	public class CPfxAssert
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
		#region Definitions

			// /////////////////////////////////////////////////////////////////////////////////
			//								C l a s s											
			/// <summary>
			/// Klasa pomocnicza - log asercji
			/// </summary>
			private class CAssertLog : CTraceCore
			{ 
				// /////////////////////////////////////////////////////////////////////////////////
				//							M e m b e r s											
			#region Members
				/// <summary>
				/// tablica konfiguracyjna logu
				/// </summary>
				private static CMessageField[] sm_oMessageFieldsArray;
			#endregion
				
				// /////////////////////////////////////////////////////////////////////////////////
				//							C o n s t r u c t o r s									
			#region Constructors

				/// <summary>
				/// Konstruktor statyczny. Inicjalizuje tablice konfiguracyjna logu.
				/// Postac logu: Data Czas ThreadId Komunikat KomunikatSzczegolowy Stos
				/// </summary>
				static CAssertLog()
				{
					sm_oMessageFieldsArray = new CMessageField[7];
					sm_oMessageFieldsArray[0] = new CMessageField(EMessageFieldType.eDate);
					sm_oMessageFieldsArray[1] = new CMessageField(EMessageFieldType.eTime);
					sm_oMessageFieldsArray[2] = new CMessageField(EMessageFieldType.eThreadId, "Thr({0,8})");
					sm_oMessageFieldsArray[3] = new CMessageField(EMessageFieldType.eUserDefined);	// message
					sm_oMessageFieldsArray[4] = new CMessageField(EMessageFieldType.eMessage);		// detailed message
					sm_oMessageFieldsArray[5] = new CMessageField(EMessageFieldType.eUserDefined);  // stack
					sm_oMessageFieldsArray[6] = new CMessageField(EMessageFieldType.eUserDefined, "\n\n{0}\n");  // separator
				}

				/// <summary>
				/// Konstruktor logu
				/// </summary>
				/// <param name="i_sDirectory">Katalog logu. Pusty lub null oznaczaja katalog aktualny</param>
				/// <param name="i_sLogName"> Nazwa logu</param>
				/// <param name="i_bAppendLog">Czy kontynuowac stary log</param>
				public CAssertLog(
					string	i_sDirectory,
					string	i_sLogName,
					bool	i_bAppendLog
				):base(
					i_sDirectory,
					"ASR",
					i_sLogName,
					null,
					null,
					0,
					sm_oMessageFieldsArray,
					i_bAppendLog,
					1
					)
				{
				}

			#endregion

				// /////////////////////////////////////////////////////////////////////////////////
				//							P u b l i c    I n t e r f a c e						
			#region PublicInterface

				/// <summary>
				/// Metoda logujaca
				/// </summary>
				/// <param name="i_sMessage">Komunikat asercji</param>
				/// <param name="i_sDetailedMessage">Komunikat szczegolowy</param>
				/// <param name="i_sStack">Stos wywolan funkcji</param>
				public void Log(
					string	i_sMessage,
					string	i_sDetailedMessage,
					string  i_sStack
				)
				{
					try
					{
						object[] oUserParams = new Object[3]{
																i_sMessage, 
																i_sStack,
																"#######################################################################"
															};
						Trace(oUserParams, i_sDetailedMessage);
					}
					catch(Exception e)
					{
						CExceptionManager.LogException(e);
					}
				}

			#endregion

				// /////////////////////////////////////////////////////////////////////////////////
				//							I n t e r n a l    F u n c t i o n s					
			#region InternalFunctions

			#endregion
			}


		/// <summary>
		/// 
		/// </summary>
		private const int def_nStackFramesToSkip = 4;

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
		#region Members
		/// <summary>
		/// Flaga sterujaca pokazywaniem komunikatu na ekranie
		/// </summary>
		private static bool sm_bShowMessage = true;

		/// <summary>
		/// Flaga sterujaca logowaniem komunikatu
		/// </summary>
		private static bool sm_bLogMessage = true;

		/// <summary>
		/// Flaga sterujaca logowaniem komunikatu
		/// </summary>
		private static bool sm_bWriteDebugMessage = true;
		
		/// <summary>
		/// Obiekt logu asercji
		/// </summary>
		private static CAssertLog sm_oLog = new CAssertLog(null, "",true);
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
		#region Constructors
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface

		/// <summary>
		/// Zmiana katalogu logu asercji
		/// </summary>
		/// <param name="i_sDirectory">sciezka</param>
		public static void ChangeLogDir(
			string i_sDirectory
			)
		{
			try
			{
				sm_oLog.ChangeLogDir(i_sDirectory);
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
		public static void ChangeLogName(
			string i_sName
			)
		{
			try
			{
				sm_oLog.ChangeLogName(i_sName);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}


		/// <summary>
		/// 
		/// </summary>
		public static bool p_bShowMessage
		{
			get
			{
				return sm_bShowMessage; 
			}
			set
			{
				sm_bShowMessage = value;
			}
		}

		/// <summary>
		/// Metoda obslugi asercji z najszerszym interfejsem
		/// </summary>
		/// <param name="i_bCondition">Warunek asercji</param>
		/// <param name="i_sMessage">Komunikat</param>
		/// <param name="i_sDetailedMessage">Komunikat dokladny</param>
		/// <param name="i_oParams">Parametry komunikatu dokladnego</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void Assert(
			bool			i_bCondition,
			string			i_sDetailedMessage,
			params object[]	i_oParams
		)
		{
			AssertPrv(
				def_nStackFramesToSkip - 2,
				i_bCondition,
				"Asercja",
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji
		/// </summary>
		/// <param name="i_bCondition">Warunek asercji</param>
		/// <param name="i_sMessage">Komunikat</param>
		/// <param name="i_sDetailedMessage">Komunikat dokladny</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void Assert(
			bool	i_bCondition,
			string	i_sDetailedMessage
		)
		{
			AssertPrv(
				def_nStackFramesToSkip - 2,
				i_bCondition,
				"Asercja",
				i_sDetailedMessage,
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - referencja nie jest null
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNull(
			object i_oObject
		)
		{
			IsNotNullPrv(
				i_oObject, 
				"",
				null
			);
		}


		/// <summary>
		/// Metoda obslugi asercji - referencja nie jest null
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNull(
			object i_oObject,
			string i_sDetailedMessage
			)
		{
			IsNotNullPrv(
				i_oObject, 
				i_sDetailedMessage,
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - referencja nie jest null
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNull(
			object			i_oObject,
			string			i_sDetailedMessage,
			params object[]	i_oParams
		)
		{
			IsNotNullPrv(
				i_oObject, 
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - referencja jest null
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNull(
			object i_oObject
		)
		{
			IsNullPrv(
				i_oObject, 
				"",
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - referencja nie jest null
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNull(
			object i_oObject,
			string i_sDetailedMessage
			)
		{
			IsNullPrv(
				i_oObject, 
				i_sDetailedMessage,
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - referencja nie jest null
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNull(
			object			i_oObject,
			string			i_sDetailedMessage,
			params object[]	i_oParams
		)
		{
			IsNullPrv(
				i_oObject, 
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - int nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegative(
			int		i_nValue
			)
		{
			IsNotNegativePrv(
				i_nValue, 
				"",
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - int nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegative(
			int		i_nValue,
			string	i_sDetailedMessage
			)
		{
			IsNotNegativePrv(
				i_nValue,
				i_sDetailedMessage,
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - int nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegative(
			int					i_nValue,
			string				i_sDetailedMessage,
			params object[]	i_oParams
		)
		{
			IsNotNegativePrv(
				i_nValue,
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - int jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsPositive(
			int		i_nValue
			)
		{
			IsPositivePrv(
				i_nValue, 
				"",
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - int jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsPositive(
			int		i_nValue,
			string	i_sDetailedMessage
			)
		{
			IsPositivePrv(
				i_nValue,
				i_sDetailedMessage,
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - int jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsPositive(
			int					i_nValue,
			string				i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			IsPositivePrv(
				i_nValue,
				i_sDetailedMessage,
				i_oParams
			);
		}


		/// <summary>
		/// Metoda obslugi asercji - long nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegative(
			long i_lValue
			)
		{
			IsNotNegativePrv(
				i_lValue, 
				"",
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - long nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegative(
			long	i_lValue,
			string	i_sDetailedMessage
			)
		{
			IsNotNegativePrv(
				i_lValue,
				i_sDetailedMessage,
				null
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - long nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegative(
			long			i_lValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			IsNotNegativePrv(
				i_lValue,
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - long jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsPositive(
			long i_lValue
			)
		{
			IsPositivePrv(
				i_lValue, 
				"",
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - long jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsPositive(
			long	i_lValue,
			string	i_sDetailedMessage
			)
		{
			IsPositivePrv(
				i_lValue,
				i_sDetailedMessage,
				null
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - long jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsPositive(
			long			i_lValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			IsPositivePrv(
				i_lValue,
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - decimal jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsPositive(
			decimal i_nfValue
			)
		{
			IsPositivePrv(
				i_nfValue,
				"",
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - long jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsPositive(
			decimal i_nfValue,
			string i_sDetailedMessage
			)
		{
			IsPositivePrv(
				i_nfValue,
				i_sDetailedMessage,
				null
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - long jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsPositive(
			decimal i_nfValue,
			string i_sDetailedMessage,
			params object[] i_oParams
			)
		{
			IsPositivePrv(
				i_nfValue,
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - short jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsPositive(
			short		i_nValue
			)
		{
			IsPositivePrv(
				i_nValue, 
				"",
				null
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - short jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsPositive(
			short	i_nValue,
			string	i_sDetailedMessage
			)
		{
			IsPositivePrv(
				i_nValue,
				i_sDetailedMessage,
				null
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - short jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsPositive(
			short					i_nValue,
			string				i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			IsPositivePrv(
				i_nValue,
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - int nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegativeOrIsEmpty(
			int	i_nValue,
			int i_nEmptyValue
			)
		{
			IsNotNegativeOrIsEmptyPrv(
				i_nValue, 
				i_nEmptyValue,
				"",
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - int nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegativeOrIsEmpty(
			int		i_nValue,
			int		i_nEmptyValue,
			string	i_sDetailedMessage
			)
		{
			IsNotNegativeOrIsEmptyPrv(
				i_nValue,
				i_nEmptyValue,
				i_sDetailedMessage,
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - int nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegativeOrIsEmpty(
			int				i_nValue,
			int				i_nEmptyValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			IsNotNegativeOrIsEmptyPrv(
				i_nValue,
				i_nEmptyValue,
				i_sDetailedMessage,
				i_oParams
			);
		}


		/// <summary>
		/// Metoda obslugi asercji - long nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegativeOrIsEmpty(
			long i_lValue,
			long i_lEmptyValue
			)
		{
			IsNotNegativeOrIsEmptyPrv(
				i_lValue, 
				i_lEmptyValue,
				"",
				null
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - long nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegativeOrIsEmpty(
			long	i_lValue,
			long	i_lEmptyValue,
			string	i_sDetailedMessage
			)
		{
			IsNotNegativeOrIsEmptyPrv(
				i_lValue,
				i_lEmptyValue,
				i_sDetailedMessage,
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - long nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegativeOrIsEmpty(
			long			i_lValue,
			long			i_lEmptyValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			IsNotNegativeOrIsEmptyPrv(
				i_lValue,
				i_lEmptyValue,
				i_sDetailedMessage,
				i_oParams
			);
		}


		/// <summary>
		/// Metoda obslugi asercji - short nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegative(
			short i_nValue
			)
		{
			IsNotNegativePrv(
				i_nValue, 
				"",
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - short nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegative(
			short	i_nValue,
			string	i_sDetailedMessage
			)
		{
			IsNotNegativePrv(
				i_nValue,
				i_sDetailedMessage,
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - short nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegative(
			short			i_nValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			IsNotNegativePrv(
				i_nValue,
				i_sDetailedMessage,
				i_oParams
			);
		}


		/// <summary>
		/// Metoda obslugi asercji - short nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegativeOrIsEmpty(
			short i_nValue,
			short i_nEmptyValue
			)
		{
			IsNotNegativeOrIsEmptyPrv(
				i_nValue, 
				i_nEmptyValue,
				"",
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - short nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegativeOrIsEmpty(
			short	i_nValue,
			short	i_nEmptyValue,
			string	i_sDetailedMessage
			)
		{
			IsNotNegativeOrIsEmptyPrv(
				i_nValue,
				i_nEmptyValue,
				i_sDetailedMessage,
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - short nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsNotNegativeOrIsEmpty(
			short			i_nValue,
			short			i_nEmptyValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			IsNotNegativeOrIsEmptyPrv(
				i_nValue,
				i_nEmptyValue,
				i_sDetailedMessage,
				i_oParams
			);
		}


		/// <summary>
		/// Metoda obslugi asercji - int jest zerem
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsZero(
			int		i_nValue
		)
		{
			IsZeroPrv(
				i_nValue, 
				"",
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - int jest zerem
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsZero(
			int		i_nValue,
			string	i_sDetailedMessage
			)
		{
			IsZeroPrv(
				i_nValue,
				i_sDetailedMessage,
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - int jest zerem
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsZero(
			int				i_nValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			IsZeroPrv(
				i_nValue,
				i_sDetailedMessage,
				i_oParams
			);
		}


		/// <summary>
		/// Metoda obslugi asercji - short jest zerem
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsZero(
			short	i_nValue
			)
		{
			IsZeroPrv(
				i_nValue, 
				"",
				null
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - short jest zerem
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsZero(
			short	i_nValue,
			string	i_sDetailedMessage
			)
		{
			IsZeroPrv(
				i_nValue,
				i_sDetailedMessage,
				null
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - short jest zerem
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void IsZero(
			short			i_nValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			IsZeroPrv(
				i_nValue,
				i_sDetailedMessage,
				i_oParams
			);		
		}


		/// <summary>
		/// Metoda obslugi asercji - lancuch nie pusty
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void StringNotEmpty(
			string i_sObject
		)
		{
			StringNotEmptyPrv(
				i_sObject,
				"",
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - lancuch nie pusty
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void StringNotEmpty(
			string i_sObject,
			string i_sDetailedMessage
			)
		{
			StringNotEmptyPrv(
				i_sObject,
				i_sDetailedMessage,
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - lancuch nie pusty
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void StringNotEmpty(
			string			i_sObject,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			StringNotEmptyPrv(
				i_sObject,
				i_sDetailedMessage,
				i_oParams
			);
		}
		
		/// <summary>
		/// Metoda obslugi asercji - Assert(false) - wejscie w niemozliwa sciezke
		/// </summary>
		/// <param name="i_sMessage">Komunikat</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void Impossible(
			string i_sDetailedMessage
		)
		{
			ImpossiblePrv(
				i_sDetailedMessage,
				null
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - Assert(false) - wejscie w niemozliwa sciezke
		/// </summary>
		/// <param name="i_sMessage">Komunikat</param>
		/// <param name="i_sDetailedMessage">Komunikat dokladny</param>
		/// <param name="i_oParams">Parametry komunikat dokladnego</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void Impossible(
			string			i_sDetailedMessage,
			params object[] i_oParams
		)
		{
			ImpossiblePrv(
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - Assert(false) - uzycie Finalizatora zamiast Dispose
		/// </summary>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		public static void Finalizer(
			object i_oObject
		)
		{
			AssertPrv(
				def_nStackFramesToSkip - 2,
				false,
				"Nie wywolano metody Dispose - uruchomil sie finalizator", 
				"Obiekt: Typ({0})", 
				i_oObject.GetType().Name
			);
		}


		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
        #region PrivateInterface

		/// <summary>
		/// Metoda obslugi asercji - referencja nie jest null
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsNotNullPrv(
			object			i_oObject,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				i_oObject != null, 
				"Pusta referencja",
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - referencja nie jest null
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsNullPrv(
			object			i_oObject,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				i_oObject == null, 
				"Referencja nie jest pusta",
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - int nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsNotNegativePrv(
			int				i_nValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				i_nValue >= 0, 
				string.Format("Podana liczba jest ujemna - Value({0})", i_nValue),
				i_sDetailedMessage,
				i_oParams
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - int jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsPositivePrv(
			int				i_nValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				i_nValue > 0, 
				string.Format("Podana liczba nie jest dodatnia - Value({0})", i_nValue),
				i_sDetailedMessage,
				i_oParams
			);
		}


		/// <summary>
		/// Metoda obslugi asercji - long nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsNotNegativePrv(
			long			i_lValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				i_lValue >= 0, 
				string.Format("Podana liczba jest ujemna - Value({0})", i_lValue),
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - long jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsPositivePrv(
			long			i_lValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				i_lValue > 0, 
				string.Format("Podana liczba nie jest dodatnia - Value({0})", i_lValue),
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - decimal jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsPositivePrv(
			decimal i_nfValue,
			string i_sDetailedMessage,
			params object[] i_oParams
			)
		{
			AssertPrv(
				i_nfValue > 0,
				string.Format("Podana liczba nie jest dodatnia - Value({0})", i_nfValue),
				i_sDetailedMessage,
				i_oParams
			);
		}


		/// <summary>
		/// Metoda obslugi asercji - short jest dodatni
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsPositivePrv(
			short			i_nValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				i_nValue > 0, 
				string.Format("Podana liczba nie jest dodatnia - Value({0})", i_nValue),
				i_sDetailedMessage,
				i_oParams
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - int nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsNotNegativeOrIsEmptyPrv(
			int				i_nValue,
			int				i_nEmptyValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				(i_nValue >= 0) || (i_nValue == i_nEmptyValue), 
				string.Format("Podana liczba jest ujemna i nie jest to wartosc pusta - Value({0}), EmptyValue({1})", i_nValue, i_nEmptyValue),
				i_sDetailedMessage,
				i_oParams
				);
		}
		
		/// <summary>
		/// Metoda obslugi asercji - int nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsNotNegativeOrIsEmptyPrv(
			short			i_nValue,
			short			i_nEmptyValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				(i_nValue >= 0) || (i_nValue == i_nEmptyValue), 
				string.Format("Podana liczba jest ujemna i nie jest to wartosc pusta - Value({0}), EmptyValue({1})", i_nValue, i_nEmptyValue),
				i_sDetailedMessage,
				i_oParams
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - long nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsNotNegativeOrIsEmptyPrv(
			long			i_lValue,
			long			i_lEmptyValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				(i_lValue >= 0) || (i_lValue == i_lEmptyValue), 
				string.Format(
					"Podana liczba jest ujemna i nie jest to wartosc pusta - Value({0}), EmptyValue({1})", 
					i_lValue, 
					i_lEmptyValue
				),
				i_sDetailedMessage,
				i_oParams
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - short nie jest ujemny
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsNotNegativePrv(
			short			i_nValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				i_nValue >= 0, 
				string.Format("Podana liczba jest ujemna - Value({0})", i_nValue),
				i_sDetailedMessage,
				i_oParams
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - short jest zerem
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsZeroPrv(
			short			i_nValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				i_nValue == 0, 
				string.Format("Podana liczba nie jest zerem - Value({0})", i_nValue),
				i_sDetailedMessage,
				i_oParams
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - int jest zerem
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void IsZeroPrv(
			int				i_nValue,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				i_nValue == 0, 
				string.Format("Podana liczba nie jest zerem - Value({0})", i_nValue),
				i_sDetailedMessage,
				i_oParams
				);
		}

		/// <summary>
		/// Metoda obslugi asercji - lancuch nie pusty
		/// </summary>
		/// <param name="i_oObject">Warunek asercji</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void StringNotEmptyPrv(
			string			i_sObject,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				CPfxVerify.StringNotEmpty(i_sObject),
				"Podany lancuch jest pusty",
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji - Assert(false) - wejscie w niemozliwa sciezke
		/// </summary>
		/// <param name="i_sMessage">Komunikat</param>
		/// <param name="i_sDetailedMessage">Komunikat dokladny</param>
		/// <param name="i_oParams">Parametry komunikat dokladnego</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void ImpossiblePrv(
			string			i_sDetailedMessage,
			params object[] i_oParams
			)
		{
			AssertPrv(
				false, 
				"Bledna sciezka wykonania programu", 
				i_sDetailedMessage,
				i_oParams
				);
		}

		/// <summary>
		/// Metoda obslugi asercji z najszerszym interfejsem
		/// </summary>
		/// <param name="i_bCondition">Warunek asercji</param>
		/// <param name="i_sMessage">Komunikat</param>
		/// <param name="i_sDetailedMessage">Komunikat dokladny</param>
		/// <param name="i_oParams">Parametry komunikatu dokladnego</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void AssertPrv(
			bool			i_bCondition,
			string			i_sMessage,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			AssertPrv(
				def_nStackFramesToSkip,
				i_bCondition,
				i_sMessage,
				i_sDetailedMessage,
				i_oParams
			);
		}

		/// <summary>
		/// Metoda obslugi asercji z najszerszym interfejsem + dodatkowo ilosc ramek 
		/// stacktrace ktore pominac przy logowaniu
		/// </summary>
		/// <param name="i_nStackFramesToSkip">Ilosc ramek stacktrace ktore mozna pominac</param>
		/// <param name="i_bCondition">Warunek asercji</param>
		/// <param name="i_sMessage">Komunikat</param>
		/// <param name="i_sDetailedMessage">Komunikat dokladny</param>
		/// <param name="i_oParams">Parametry komunikatu dokladnego</param>
		[Conditional("DEBUG"), Conditional("PFX_USE_ASSERT")]
		private static void AssertPrv(
			int				i_nStackFramesToSkip,
			bool			i_bCondition,
			string			i_sMessage,
			string			i_sDetailedMessage,
			params object[]	i_oParams
			)
		{
			try
			{
				if (i_bCondition)
					return;

				if (!sm_bLogMessage && !sm_bShowMessage && !sm_bWriteDebugMessage)
					return;

				string sDetMsg;
				if (i_oParams != null)
					sDetMsg = string.Format(i_sDetailedMessage, i_oParams);
				else
					sDetMsg = i_sDetailedMessage;
                
				if (sm_bLogMessage || sm_bShowMessage)
				{
					StackTrace	oStack;
					string		sLocation;
					string		sStack;

					oStack		= new StackTrace(i_nStackFramesToSkip, true);
					sLocation	= GetLocation(oStack);
					sStack		= oStack.ToString();
					if (sm_bLogMessage)
					{
						sm_oLog.Log(i_sMessage, sDetMsg + sLocation, sStack);
					}

					if (sm_bShowMessage)
					{
						Debug.Assert(i_bCondition, i_sMessage, sDetMsg + sLocation);
					}
				}
				
				if (sm_bWriteDebugMessage)
					Debug.WriteLine(sDetMsg, i_sMessage);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
			}
		}

		/// <summary>
		/// Metoda wyciagajaca dane lokalizacji asercji w kodzie
		/// </summary>
		/// <param name="i_oStack">obiekt StackTrace</param>
		/// <returns>string opisujacy lokalizacje</returns>
		private static string GetLocation(
			StackTrace	i_oStack
		)
		{
			try
			{
				StackFrame oFrame	= i_oStack.GetFrame(0);
				
				if (oFrame == null)
					return "(nieznana lokalizaja)";
				
				MethodBase oMethod	= oFrame.GetMethod();
				
				if (oMethod == null)
					return "(nieznana lokalizaja)";
							
				return string.Format("\nklasa:\t{0}\nmetoda:\t{1}\nplik:\t{2}\nl/k:\t{3}/{4}\n",
					oMethod.DeclaringType.Name,
					oMethod.Name,
					oFrame.GetFileName(),
					oFrame.GetFileLineNumber(),
					oFrame.GetFileColumnNumber()
					);
			}
			catch(Exception e)
			{
				CExceptionManager.LogException(e);
				return "(nieznana lokalizaja)";
			}
		}

		#endregion
	}
}
