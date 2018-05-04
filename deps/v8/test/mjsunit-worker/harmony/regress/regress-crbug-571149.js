// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/harmony/regress/regress-crbug-571149.js");
	}
}
(function(a = 0){
  var x;  // allocated in a var block, due to use of default parameter
  (function() { return !x })();
})();

(function({a}){
  var x;  // allocated in a var block, due to use of parameter destructuring
  (function() { return !x })();
})({});

(function(...a){
  var x;  // allocated in a var block, due to use of rest parameter
  (function() { return !x })();
})();
