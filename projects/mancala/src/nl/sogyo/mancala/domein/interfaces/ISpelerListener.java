package nl.sogyo.mancala.domein.interfaces;

/**
 * @author Peter Postma
 */
public interface ISpelerListener
{
	void komUpdateEvent(IKom kom, boolean isSlag);
	void spelAfgelopenEvent();
	void spelerWisselBeurtEvent(ISpeler speler);
}
