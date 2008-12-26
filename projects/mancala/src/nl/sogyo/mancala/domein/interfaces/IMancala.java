package nl.sogyo.mancala.domein.interfaces;

/**
 * @author Peter Postma
 */
public interface IMancala
{
	void addListener(ISpelerListener listener);
	void removeListener(ISpelerListener listener);

	ISpeler getSpelerAanZet();
	ISpeler getWinnaar();

	ISpeler getSpeler1();
	ISpeler getSpeler2();
}
