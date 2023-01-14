using System.Resources;

namespace net.jurasz.tools
{
	/// <summary>
	/// 
	/// </summary>
	public enum ENoMessageAction
	{
		eReturnNull			= 0,
		eReturnEmpty		= 1,
		eReturnId			= 2,
		eReturnUserString	= 3
	}

	// /////////////////////////////////////////////////////////////////////////////////
	//								C l a s s											
	/// <summary>
	/// interfejs klasy opakowujacej ResourceManager'a
	/// </summary>
	public interface IResourceManagerEx
	{
        ///////////////////////////////////////////////////////////////////////////////////
        // D E F I N I T I O N S (statics, consts etc)
		#region Definitions

		#endregion

        ///////////////////////////////////////////////////////////////////////////////////
        // P U B L I C  I N T E R F A C E (public methods etc)
		#region PublicInterface

		/// <summary>
		/// 
		/// </summary>
		bool p_bCascadeResource { get; set; }


		/// <summary>
		/// 
		/// </summary>
		ResourceManager p_oResourceManager { get; }

		/// <summary>
		/// 
		/// </summary>
		ENoMessageAction p_eNoMsgAction { get; set; }

		/// <summary>
		/// 
		/// </summary>
		bool p_bEnableDTR { set; }

		/// <summary>
		/// Wyciaga ³añcuch z zasobów
		/// </summary>
		/// <returns></returns>
		string GetResourceString(
			string i_sMessageId
			);

		#endregion
	}
}