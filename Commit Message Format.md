This page documents our source control commit message format.

This format is also documented in [Contributions.txt](https://edk2.svn.sourceforge.net/svnroot/edk2/trunk/edk2/MdePkg/Contributions.txt) which may be available in the source tree as well.

Use this format for commit messages, and when providing the log message for a patch.

# Commit message format

<pre>
Pkg-Module: Brief-single-line-summary

Full-commit-message

Contributed-under: TianoCore Contribution Agreement 1.0
Signed-off-by: Contributor Name &lt;contributor@email.server&gt;
</pre>

Where:
* Pkg-Module is the EDK II Package and optionally the Module
* Brief-single-line-summary is a short summary of the change
* The length of 'Pkg-Module: Brief-single-line-summary' should not exceed 70 characters
* blank-line is an empty line
* Full-commit-message is the full message describing the change
 * Line length should be limited to 70 characters when possible
* Signatures is one or more lines with signatures.  Please see the [[Commit Signature Format]] page for more information.
* The entire log message should use only standard ASCII text characters

An example would be:
<Pre>
Package/Module: Short one line description of change

Several lines of
description for the
change.

Contributed-under: TianoCore Contribution Agreement 1.0
Signed-off-by: Contributor Name &lt;contributor@email.server&gt;
Reviewed-by: Reviewer Name &lt;reviewer@reviewer-email.server&gt;
</Pre>

# See Also
* [[Code-Style]]
* [[Oneline Release Notes Generation]]