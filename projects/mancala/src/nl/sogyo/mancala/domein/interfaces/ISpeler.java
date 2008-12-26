package nl.sogyo.mancala.domein.interfaces;

import java.util.List;

/**
 * @author Peter Postma
 */
public interface ISpeler
{
	void addListener(ISpelerListener listener);
	void removeListener(ISpelerListener listener);

	String getNaam();

	IKom getKomOpPositie(int positie);
	IKom getKalaha();
	List<IKom> getKommetjes();

	int getTotaalAantalStenen();

	boolean kanZetDoen();
}
