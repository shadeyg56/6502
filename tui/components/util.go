package components

import "charm.land/bubbles/v2/key"

func RemoveShortcut(binding *key.Binding, keyToRemove string) {
	currentKeys := binding.Keys()
	var newKeys []string

	for _, k := range currentKeys {
		if k != keyToRemove {
			newKeys = append(newKeys, k)
		}
	}

	binding.SetKeys(newKeys...)
}
