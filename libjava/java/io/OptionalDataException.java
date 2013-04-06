/* Copyright (C) 2000  Free Software Foundation

   This file is part of libgcj.

This software is copyrighted work licensed under the terms of the
Libgcj License.  Please consult the file "LIBGCJ_LICENSE" for
details.  */

package java.io;

/**
 * @author Warren Levy <warrenl@cygnus.com>
 * @date February 7, 2000.
 */

/* Written using on-line Java Platform 1.2 API Specification.
 * Status:  Believed complete and correct.
 */

public class OptionalDataException extends ObjectStreamException
{
  // FIXME: Need to set these fields per the doc in a constructor.
  public boolean eof;
  public int length;

  // FIXME: This can probably go away once the right signatures of
  // these package private constructors is determined.
  private static final long serialVersionUID = -8011121865681257820L;

  OptionalDataException()
  {
    super();
  }

  OptionalDataException(String msg)
  {
    super(msg);
  }
}
