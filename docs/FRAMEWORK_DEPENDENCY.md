# Framework dependency

Normal Umicom OS profiles require Framework. Recovery does not.

This preserves two properties simultaneously:

1. one common runtime/UI implementation for all Umicom applications;
2. a boot/recovery path that cannot be broken by an application-framework
   package failure.
