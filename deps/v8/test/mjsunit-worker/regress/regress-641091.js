// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-641091.js");
	}
}
assertEquals(["🍤", "🍤"],
             '🍤🍦🍋ππ🍋🍦🍤'.match(/🍤/ug));

assertEquals(["🍤", "🍦", "🍦", "🍤"],
             '🍤🍦🍋ππ🍋🍦🍤'.match(/🍤|🍦/ug));

assertEquals(["🍤", "🍦", "🍋", "🍋", "🍦", "🍤"],
             '🍤🍦🍋ππ🍋🍦🍤'.match(/🍤|🍦|🍋/ug));

assertEquals(["🍤", "🍦", "🍋", "π", "π", "🍋", "🍦", "🍤"],
             '🍤🍦🍋ππ🍋🍦🍤'.match(/🍤|🍦|π|🍋/ug));
