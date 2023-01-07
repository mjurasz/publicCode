using System;

namespace net.jurasz.tools
{
	// /////////////////////////////////////////////////////////////////////////////////
	//								C l a s s											
	/// <summary>
	/// Klasa obslugi sprawdzania poprawnosci danych
	/// </summary>
	public class CPfxVerify
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
		#region Definitions
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // M E M B E R S (variables etc)
		#region Members
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // C O N S T R U C T O R S (constructors etc)
		#region Constructors
		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface


		/// <summary>
		/// 
		/// </summary>

		/// <summary>
		/// Sprawdza, czy liczba jest nieujemna lub rowna wartosci domyslnej
		/// </summary>
		/// <param name="i_nValue">wartosc sprawdzana</param>
		/// <param name="i_nEmptyValue">Wartosc domyslna</param>
		/// <returns></returns>
		public static bool IsNotNegativeOrIsEmpty(
			int	i_nValue,
			int i_nEmptyValue
			)
		{
			if (i_nValue >= 0)
				return true;
			return i_nValue == i_nEmptyValue;
		}

		/// <summary>
		/// Sprawdza, czy liczba jest nieujemna lub rowna wartosci domyslnej
		/// </summary>
		/// <param name="i_nValue">wartosc sprawdzana</param>
		/// <param name="i_nEmptyValue">Wartosc domyslna</param>
		/// <returns></returns>
		public static bool IsNotNegativeOrIsEmpty(
			short i_nValue,
			short i_nEmptyValue
			)
		{
			if (i_nValue >= 0)
				return true;
			return i_nValue == i_nEmptyValue;
		}

		/// <summary>
		/// Sprawdza, czy lancuch nie jest pusty
		/// </summary>
		/// <param name="i_sObject">Warunek asercji</param>
		public static bool StringNotEmpty(
			string i_sObject
			)
		{
			if (i_sObject == null)
				return false;
			return i_sObject.Length != 0;
		}

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P R I V A T E  I N T E R F A C E (private, protected methods etc)
		#region PrivateInterface
		#endregion
	}
}
