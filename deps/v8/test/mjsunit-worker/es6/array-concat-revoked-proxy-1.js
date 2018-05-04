// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/array-concat-revoked-proxy-1.js");
	}
}
(function testConcatRevokedProxyToArrayInPrototype() {
  "use strict";
  var handler = {
    get(_, name) {
      if (name === Symbol.isConcatSpreadable) {
        p.revoke();
      }
      return target[name];
    }
  }

  var p = Proxy.revocable([], handler);
  var target = { __proto__: p.proxy };
  assertThrows(function() { [].concat(target); }, TypeError);
})();
