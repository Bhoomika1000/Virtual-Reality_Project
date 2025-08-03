using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class EscapeRoomSpawner : MonoBehaviour
{
    [Header("Spawn Settings")]
    public Transform refCube;

    [Header("Offset Settings")]
    public Vector3 spawnOffset = new Vector3(0, 2, 0);

    void Start()
    {
        // Find player and move to spawn position
        GameObject player = GameObject.FindGameObjectWithTag("Player");
        if (player != null && refCube != null)
        {
            player.transform.position = refCube.position + spawnOffset;
        }
    }
}







