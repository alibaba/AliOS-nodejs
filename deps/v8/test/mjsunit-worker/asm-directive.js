// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//asm-directive.js");
	}
}
Realm.eval(Realm.current(), '"use asm"');
function f() { "use asm" }
() => "use asm"
if (true) "use asm"
with ({}) "use asm"
try { } catch (e) { "use asm" }
Realm.eval(Realm.current(), 'eval(\'"use asm"\')');
