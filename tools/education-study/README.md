# Education study host

This directory composes the Framework's native study tools. It does not contain
another catalogue, learner database, quiz engine or kernel feature.

Use a separate build directory and point `UMICOM_OS_FRAMEWORK_SOURCE` at the
canonical Framework checkout. See the source guide at
`framework/docs/learning/EDUCATION_STUDY.html` and the Framework SDK example.

The default is this OS repository's `framework` submodule. The source batch must
be merged there by aligning the submodule to the published Framework commit,
not by maintaining a separate implementation. A successful host test is not an
Umicom OS boot result.
