// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-417508.js");
	}
}
function foo(x) {
  var k = "value";
  return x[k] = 1;
}
var obj = {};
Object.defineProperty(obj, "value", {set: function(x) { throw "nope"; }});
try { foo(obj); } catch(e) {}
try { foo(obj); } catch(e) {}
%OptimizeFunctionOnNextCall(foo);
try { foo(obj); } catch(e) {}

function bar(x) {
  var k = "value";
  return (x[k] = 1) ? "ok" : "nope";
}
var obj2 = {};
Object.defineProperty(obj2, "value",
    {set: function(x) { throw "nope"; return true; } });

try { bar(obj2); } catch(e) {}
try { bar(obj2); } catch(e) {}
%OptimizeFunctionOnNextCall(bar);
try { bar(obj2); } catch(e) {}
