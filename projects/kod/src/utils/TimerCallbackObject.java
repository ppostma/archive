/* $Id: TimerCallbackObject.java,v 1.1 2005-08-28 15:05:08 peter Exp $ */

/**
 * Kiss of Death
 * TimerCallbackObject.java
 * 
 * Specifies implementation details for subclasses of Timer callback objects.
 */

package src.utils;

public interface TimerCallbackObject
{
    void ready();
    void error(Exception e);
}

