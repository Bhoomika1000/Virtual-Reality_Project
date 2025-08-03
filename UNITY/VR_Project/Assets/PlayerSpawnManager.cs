using System.Collections;
using System.Collections.Generic;
using UnityEngine;


public class PlayerSpawnManager : MonoBehaviour
{
    [Header("Player Settings")]
    public GameObject player; // Assign your player GameObject

    [Header("Spawn Position (Based on PlayerReference Cube)")]
    public Vector3 spawnPosition = new Vector3(525.83f, 6.93f, 499.03f);
    public Vector3 spawnRotation = new Vector3(0f, 179.949f, 0f);

    [Header("First Person Camera Settings")]
    public Camera mainCamera;
    public Vector3 cameraOffset = new Vector3(0f, 1.6f, 0f); // Inside player at eye level
    public bool attachCameraToPlayer = true;

    [Header("SimpleMovement Settings")]
    public float playerSpeed = 5f;
    public float mouseSensitivity = 2f;
    public float playerHeight = 2f;
    public LayerMask groundLayer = -1;

    [Header("Player Body Settings")]
    public bool hidePlayerBody = true; // Added this missing variable

    [Header("Game Start Settings")]
    public bool resetOnStart = true;
    public bool resetOnAwake = true;
    public bool addMovementScript = true;

    void Awake()
    {
        if (resetOnAwake)
        {
            SetPlayerSpawnPosition();
            SetCameraPosition();
            SetupPlayerMovement();
        }
    }

    void Start()
    {
        if (resetOnStart)
        {
            SetPlayerSpawnPosition();
            SetCameraPosition();
            SetupPlayerMovement();
        }
    }

    public void SetPlayerSpawnPosition()
    {
        if (player != null)
        {
            // Use raycast to find proper ground height at spawn position
            Vector3 finalSpawnPos = spawnPosition;

            RaycastHit hit;
            if (Physics.Raycast(spawnPosition + Vector3.up * 100f, Vector3.down, out hit, 200f, groundLayer))
            {
                // Set spawn position to ground height + player height
                finalSpawnPos.y = hit.point.y + playerHeight;
                Debug.Log("Ground found at: " + hit.point.y + ", spawning player at: " + finalSpawnPos.y);
            }
            else
            {
                Debug.LogWarning("No ground found at spawn position, using default Y position");
            }

            // Set player position and rotation
            player.transform.position = finalSpawnPos;
            player.transform.rotation = Quaternion.Euler(spawnRotation);

            // Hide player body for first person view
            if (hidePlayerBody)
            {
                MeshRenderer playerMesh = player.GetComponent<MeshRenderer>();
                if (playerMesh != null)
                {
                    playerMesh.enabled = false; // Hide the capsule visual
                }
            }

            Debug.Log("Player spawned at: " + finalSpawnPos);
        }
        else
        {
            Debug.LogWarning("Player GameObject not assigned in PlayerSpawnManager!");
        }
    }

    public void SetupPlayerMovement()
    {
        if (player != null && addMovementScript)
        {
            // Add SimpleMovement script if it doesn't exist
            SimpleMovement movement = player.GetComponent<SimpleMovement>();
            if (movement == null)
            {
                movement = player.AddComponent<SimpleMovement>();
            }

            // Configure movement settings
            movement.speed = playerSpeed;
            movement.mouseSensitivity = mouseSensitivity;
            movement.playerHeight = playerHeight;
            movement.groundLayer = groundLayer;

            Debug.Log("SimpleMovement script configured");
        }
    }

    public void SetCameraPosition()
    {
        if (mainCamera != null && player != null)
        {
            if (attachCameraToPlayer)
            {
                // Make camera child of player for first person
                mainCamera.transform.SetParent(player.transform);
                mainCamera.transform.localPosition = cameraOffset; // Eye level
                mainCamera.transform.localRotation = Quaternion.identity;
            }

            // Set camera properties for first person
            mainCamera.clearFlags = CameraClearFlags.Skybox;
            mainCamera.fieldOfView = 75f; // Wider FOV for first person
            mainCamera.nearClipPlane = 0.1f; // Closer near plane
            mainCamera.farClipPlane = 1000f;

            Debug.Log("First person camera attached to player");
        }
        else
        {
            Debug.LogWarning("Main Camera or Player not assigned in PlayerSpawnManager!");
        }
    }

    // Call this method to respawn player (useful for game restart)
    public void RespawnPlayer()
    {
        SetPlayerSpawnPosition();
        SetCameraPosition();
        SetupPlayerMovement();
    }

    // Update spawn position in inspector and apply immediately
    [ContextMenu("Apply Spawn Position")]
    public void ApplySpawnPosition()
    {
        SetPlayerSpawnPosition();
    }

    [ContextMenu("Apply Camera Position")]
    public void ApplyCameraPosition()
    {
        SetCameraPosition();
    }

    [ContextMenu("Setup Movement Script")]
    public void ApplyMovementSetup()
    {
        SetupPlayerMovement();
    }

    [ContextMenu("Set Current Player Position as Spawn")]
    public void SetCurrentPositionAsSpawn()
    {
        if (player != null)
        {
            spawnPosition = player.transform.position;
            spawnRotation = player.transform.rotation.eulerAngles;
            Debug.Log("Spawn position updated to: " + spawnPosition);
        }
    }

    [ContextMenu("Test Ground Raycast at Spawn")]
    public void TestGroundRaycast()
    {
        RaycastHit hit;
        Vector3 rayStart = spawnPosition + Vector3.up * 100f;

        if (Physics.Raycast(rayStart, Vector3.down, out hit, 200f, groundLayer))
        {
            Debug.Log("Ground found at: " + hit.point + " | Distance: " + hit.distance);
            Debug.Log("Ground object: " + hit.collider.name);

            // Draw debug line in scene view
            Debug.DrawLine(rayStart, hit.point, Color.green, 5f);
        }
        else
        {
            Debug.LogWarning("No ground found! Check LayerMask and terrain setup");
            Debug.DrawLine(rayStart, rayStart + Vector3.down * 200f, Color.red, 5f);
        }
    }
}
