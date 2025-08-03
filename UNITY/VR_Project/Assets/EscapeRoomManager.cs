using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class EscapeRoomManager : MonoBehaviour
{
    [Header("Puzzle 1: Sibling Order Buttons")]
    public GameObject redButton; // Lucy
    public GameObject greenButton; // Susan
    public GameObject yellowButton; // Peter
    public GameObject blueButton; // Edmund

    [Header("Puzzle 2: Second Clue Board")]
    public GameObject puzzle2Board; // Your "Puzzle2" cube object
    public GameObject riddle2Text; // Your "Riddle2" text object

    [Header("Puzzle 3: Crystal")]
    public GameObject crystal; // Your "Crystal" object - this IS the key!

    [Header("UI Text (Optional)")]
    public TextMeshProUGUI riddleText; // Create a UI Text for riddles
    public TextMeshProUGUI statusText; // Create a UI Text for status

    [Header("Exit")]
    public GameObject exitDoor; // Your Wall_gateR

    // Private variables
    private string[] correctOrder = { "Lucy", "Edmund", "Susan", "Peter" };
    private int currentButtonIndex = 0;
    private bool puzzle1Complete = false;
    private bool puzzle2Complete = false;
    private bool escapeRoomComplete = false;

    void Start()
    {
        SetupRoom();
        ShowInitialInstructions();
    }

    void SetupRoom()
    {
        // Set initial button colors
        SetButtonColor(redButton, Color.red);
        SetButtonColor(greenButton, Color.green);
        SetButtonColor(yellowButton, Color.yellow);
        SetButtonColor(blueButton, Color.blue);

        // Hide the second clue board initially
        if (puzzle2Board != null)
            puzzle2Board.SetActive(false);

        // Hide the crystal initially
        if (crystal != null)
            crystal.SetActive(false);

        // Lock the exit door (red = locked)
        if (exitDoor != null)
            SetButtonColor(exitDoor, Color.red);

        Debug.Log("Escape Room Setup Complete!");
    }

    void ShowInitialInstructions()
    {
        if (riddleText != null)
            riddleText.text = "Welcome to the Escape Room!\n\nFirst, press the sibling buttons in the correct order:\nLucy → Edmund → Susan → Peter";

        if (statusText != null)
            statusText.text = "Press buttons in order: Lucy → Edmund → Susan → Peter";
    }

    // Call this function from ButtonInteraction script
    public void ButtonPressed(string buttonName)
    {
        if (puzzle1Complete) return; // Don't allow more button presses

        if (buttonName == correctOrder[currentButtonIndex])
        {
            Debug.Log("Correct! Button: " + buttonName);
            currentButtonIndex++;

            // Change button color to show it's been pressed
            GameObject pressedButton = GetButtonByName(buttonName);
            if (pressedButton != null)
                SetButtonColor(pressedButton, Color.white);

            if (currentButtonIndex >= correctOrder.Length)
            {
                CompletePuzzle1();
            }
            else
            {
                if (statusText != null)
                    statusText.text = "Correct! Next: " + correctOrder[currentButtonIndex];
            }
        }
        else
        {
            Debug.Log("Wrong order! Resetting...");
            ResetButtonPuzzle();
        }
    }

    void CompletePuzzle1()
    {
        puzzle1Complete = true;
        Debug.Log("Puzzle 1 Complete! Second clue board and crystal are now visible.");

        if (riddleText != null)
            riddleText.text = "Excellent! Button puzzle complete!\n\nThe crystal has appeared in the bookshelf!\nThrow it at the door to escape!";

        if (statusText != null)
            statusText.text = "Crystal is now visible! Throw it at the door!";

        // Show the second clue board
        if (puzzle2Board != null)
        {
            puzzle2Board.SetActive(true);
            Debug.Log("Second clue board activated!");
        }

        // AUTOMATICALLY SHOW AND ACTIVATE THE CRYSTAL
        if (crystal != null)
        {
            crystal.SetActive(true);
            Debug.Log("Crystal is now visible and ready to be thrown!");

            // Make crystal glow to indicate it's active
            Light crystalLight = crystal.GetComponent<Light>();
            if (crystalLight == null)
            {
                crystalLight = crystal.AddComponent<Light>();
            }
            crystalLight.color = Color.cyan;
            crystalLight.intensity = 2f;
            crystalLight.range = 10f;

            // Mark puzzle 2 as complete since crystal is now available
            puzzle2Complete = true;
        }

        // Set the riddle text content
        if (riddle2Text != null)
        {
            TextMeshPro textComponent = riddle2Text.GetComponent<TextMeshPro>();
            if (textComponent != null)
            {
                textComponent.text = "Well done! The siblings' order is true,\nFind the crystal among the tools of wisdom here.\n\nThrow it at the wall of the first rhyme you read\nAnd you will soon be freed.";
            }
        }
    }

    // This function is now optional since crystal appears automatically
    public void SecondClueRead()
    {
        if (!puzzle1Complete)
        {
            if (statusText != null)
                statusText.text = "Complete the button puzzle first!";
            return;
        }

        Debug.Log("Second clue read!");

        if (riddleText != null)
            riddleText.text = "You've read the clue!\n\nClick and drag the crystal to throw it at the door!";

        if (statusText != null)
            statusText.text = "Throw the crystal at the door to escape!";
    }

    // This function can be called when the crystal successfully opens the door
    public void CrystalHitDoor()
    {
        if (!puzzle2Complete)
        {
            if (statusText != null)
                statusText.text = "Something's not right...";
            return;
        }

        Debug.Log("Crystal hit the door! Escape room complete!");
        escapeRoomComplete = true;

        if (riddleText != null)
            riddleText.text = "🎉 PERFECT THROW! 🎉\n\nThe door is opening!\nYou have successfully escaped!";

        if (statusText != null)
            statusText.text = "ESCAPE ROOM COMPLETE! WELL DONE!";

        // Make crystal glow brighter to celebrate
        if (crystal != null)
        {
            Light crystalLight = crystal.GetComponent<Light>();
            if (crystalLight != null)
            {
                crystalLight.color = Color.green;
                crystalLight.intensity = 5f;
            }
        }
    }

    // Call this from CrystalInteraction script (if you still want touch interaction)
    public void CrystalTouched()
    {
        if (!puzzle2Complete)
        {
            if (statusText != null)
                statusText.text = "Complete the first puzzle!";
            return;
        }

        Debug.Log("Crystal touched!");

        if (riddleText != null)
            riddleText.text = "Crystal is ready to throw!\n\nClick and drag to aim, release to throw!";

        if (statusText != null)
            statusText.text = "Throw the crystal at the door!";
    }

    // Call this from DoorInteraction script
    public void DoorTouched()
    {
        if (!escapeRoomComplete)
        {
            if (statusText != null)
                statusText.text = "Throw the crystal at the door first!";
            return;
        }

        Debug.Log("Door touched! Player escapes!");

        if (riddleText != null)
            riddleText.text = "🎉 YOU ESCAPED! CONGRATULATIONS! 🎉";

        if (statusText != null)
            statusText.text = "FREEDOM ACHIEVED!";

        // Optional: Load next scene or restart
        // SceneManager.LoadScene("NextScene");
    }

    void ResetButtonPuzzle()
    {
        currentButtonIndex = 0;

        // Reset button colors
        SetButtonColor(redButton, Color.red);
        SetButtonColor(greenButton, Color.green);
        SetButtonColor(yellowButton, Color.yellow);
        SetButtonColor(blueButton, Color.blue);

        if (statusText != null)
            statusText.text = "Wrong order! Try again: Lucy → Edmund → Susan → Peter";
    }

    GameObject GetButtonByName(string name)
    {
        switch (name)
        {
            case "Lucy": return redButton;
            case "Edmund": return blueButton;
            case "Susan": return greenButton;
            case "Peter": return yellowButton;
            default: return null;
        }
    }

    void SetButtonColor(GameObject obj, Color color)
    {
        if (obj != null)
        {
            Renderer renderer = obj.GetComponent<Renderer>();
            if (renderer != null)
            {
                renderer.material.color = color;
            }
        }
    }
}