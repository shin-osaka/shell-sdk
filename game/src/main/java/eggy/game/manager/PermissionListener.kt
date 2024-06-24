package eggy.game.manager

interface PermissionListener {
    fun permissionGranted(permission: Array<String?>)
    fun permissionDenied(permission: Array<String?>)
}