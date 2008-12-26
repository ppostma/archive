package nl.sogyo.mancala.domein.excepties;

/**
 * @author Peter Postma
 */
public class MancalaException extends Exception
{
	public MancalaException()
	{
		super();
	}

	public MancalaException(String message)
	{
		super(message);
	}

	public MancalaException(Throwable cause)
	{
		super(cause);
	}

	public MancalaException(String message, Throwable cause)
	{
		super(message, cause);
	}
}
