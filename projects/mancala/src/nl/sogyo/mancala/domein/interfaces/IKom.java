package nl.sogyo.mancala.domein.interfaces;

import java.util.List;

import nl.sogyo.mancala.domein.excepties.MancalaException;

/**
 * @author Peter Postma
 */
public interface IKom
{
	void speel() throws MancalaException;

	IKom getKomOpPositie(int positie);
	IKom getKalaha();
	List<IKom> getKommetjes();

	int getAantalStenen();
	int getTotaalAantalStenen();

	boolean kanSpelen();

	ISpeler getEigenaar();
}
